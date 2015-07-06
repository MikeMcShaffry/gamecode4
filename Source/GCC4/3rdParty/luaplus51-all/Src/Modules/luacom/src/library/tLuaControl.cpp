/*
 * tLuaControl.cpp
 *
 */

#include <ole2.h>
#include <oleidl.h>
#include <ocidl.h>

// for MINGW and/or Wine
#include <olectl.h>
#ifndef VIEWSTATUS_OPAQUE
#define VIEWSTATUS_OPAQUE 1
#endif
#ifndef POINTERINACTIVE_ACTIVATEONENTRY
#define POINTERINACTIVE_ACTIVATEONENTRY 1
#endif

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include <lauxlib.h>
#include "LuaCompat.h"
}

#include "luacom_internal.h"
#include "tLuaDispatch.h"
#include "tLuaControl.h"

const float  HIMETRIC_PER_PIXEL(26.4583333333f);

const OLEVERB verbs[] = {
    { OLEIVERB_SHOW,            0, 0, 0},
    { OLEIVERB_HIDE,            0, 0, 0},
    { OLEIVERB_INPLACEACTIVATE, 0, 0, 0},
    { OLEIVERB_PRIMARY,         0, 0, 0},
    { OLEIVERB_UIACTIVATE,      0, 0, 0}
};

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#define ASSERT(fTest, szMsg)                                \
    if (!(fTest))  {                                        \
        static char szMsgCode[] = szMsg;                    \
        static char szAssert[] = #fTest;                    \
        DisplayAssert(szMsgCode, szAssert, __FILE__, __LINE__); \
    }

#define FAIL(szMsg)                                         \
        { static char szMsgCode[] = szMsg;                    \
        DisplayAssert(szMsgCode, "FAIL", __FILE__, __LINE__); }

// macro that checks a pointer for validity on input
//
#define CHECK_POINTER(val) if (!(val) || IsBadWritePtr((void *)(val), sizeof(void *))) return E_POINTER

#define RELEASE_OBJECT(ptr)    if (ptr) { IUnknown *pUnk = (ptr); (ptr) = NULL; pUnk->Release(); }
#define QUICK_RELEASE(ptr)     if (ptr) ((IUnknown *)ptr)->Release();
#define ADDREF_OBJECT(ptr)     if (ptr) (ptr)->AddRef()

#define RETURN_ON_FAILURE(hr) if (FAILED(hr)) return hr
#define CLEANUP_ON_FAILURE(hr) if (FAILED(hr)) goto CleanUp

MIDL_DEFINE_GUID(CLSID, CLSID_Teste,0x687362C8,0x00D6,0x4eff,0x92,0x07,0xDD,0xB2,0x2E,0xE2,0x30,0x6D);

//=--------------------------------------------------------------------------=
// Private Constants
//---------------------------------------------------------------------------=
//
static const char szFormat[]  = "%s\nFile %s, Line %d";
static const char szFormat2[] = "%s\n%s\nFile %s, Line %d";

#define _SERVERNAME_ "ActiveX Framework"

static const char szTitle[]  = _SERVERNAME_ " Assertion  (Abort = UAE, Retry = INT 3, Ignore = Continue)";

//=--------------------------------------------------------------------------=
// Local functions
//=--------------------------------------------------------------------------=
int NEAR _IdMsgBox(LPSTR pszText, LPCSTR pszTitle, UINT mbFlags);

//=--------------------------------------------------------------------------=
// DisplayAssert
//=--------------------------------------------------------------------------=
// Display an assert message box with the given pszMsg, pszAssert, source
// file name, and line number. The resulting message box has Abort, Retry,
// Ignore buttons with Abort as the default.  Abort does a FatalAppExit;
// Retry does an int 3 then returns; Ignore just returns.
//
VOID DisplayAssert
(
    LPSTR	 pszMsg,
    LPSTR	 pszAssert,
    LPSTR	 pszFile,
    UINT	 line
)
{
    char	szMsg[250];
    LPSTR	lpszText;

    lpszText = pszMsg;		// Assume no file & line # info

    // If C file assert, where you've got a file name and a line #
    //
    if (pszFile) {

        // Then format the assert nicely
        //
        wsprintf(szMsg, szFormat, (pszMsg&&*pszMsg) ? pszMsg : pszAssert, pszFile, line);
        lpszText = szMsg;
    }

    // Put up a dialog box
    //
    switch (_IdMsgBox(lpszText, szTitle, MB_ICONHAND|MB_ABORTRETRYIGNORE|MB_SYSTEMMODAL)) {
        case IDABORT:
            FatalAppExit(0, lpszText);
            return;

        case IDRETRY:
            // call the win32 api to break us.
            //
            DebugBreak();
            return;
    }

    return;
}


//=---------------------------------------------------------------------------=
// Beefed-up version of WinMessageBox.
//=---------------------------------------------------------------------------=
//
int NEAR _IdMsgBox
(
    LPSTR	pszText,
    LPCSTR	pszTitle,
    UINT	mbFlags
)
{
    HWND hwndActive;
    int  id;

    hwndActive = GetActiveWindow();

    id = MessageBox(hwndActive, pszText, pszTitle, mbFlags);

    return id;
}

void PixelToHimetric(const SIZEL* pixelSize, SIZEL* himetricSize) {
	SIZEL ret;
	ret.cx = static_cast<long>(pixelSize->cx * HIMETRIC_PER_PIXEL + 0.5);
	ret.cy = static_cast<long>(pixelSize->cy * HIMETRIC_PER_PIXEL + 0.5);
	*himetricSize = ret;
}

void HimetricToPixel(const SIZEL* himetricSize, SIZEL* pixelSize) {
	SIZEL ret;
	ret.cx = static_cast<long>(himetricSize->cx / HIMETRIC_PER_PIXEL + 0.5);
	ret.cy = static_cast<long>(himetricSize->cy / HIMETRIC_PER_PIXEL + 0.5);
	*pixelSize = ret;
}

//=--------------------------------------------------------------------------=
// GetParkingWindow
//=--------------------------------------------------------------------------=
// creates the global parking window that we'll use to parent things, or
// returns the already existing one
//
// Output:
//    HWND                - our parking window
//
// Notes:
//
HWND GetParkingWindow(void)
{
    WNDCLASS wndclass;

    // crit sect this creation for apartment threading support.
    //
    if (g_hwndParking)
        goto CleanUp;

    ZeroMemory(&wndclass, sizeof(wndclass));
    wndclass.lpfnWndProc = DefWindowProc;
    wndclass.hInstance   = g_hInstance;
    wndclass.lpszClassName = "CtlFrameWork_Parking";

    if (!RegisterClass(&wndclass)) {
        FAIL("Couldn't Register Parking Window Class!");
        goto CleanUp;
    }

    g_hwndParking = CreateWindow("CtlFrameWork_Parking", NULL, WS_POPUP, 0, 0, 0, 0, NULL, NULL, g_hInstance, NULL);

    ASSERT(g_hwndParking, "Couldn't Create Global parking window!!");


  CleanUp:
    return g_hwndParking;
}

tLuaControl::tLuaControl(lua_State* L, ITypeInfo *pTypeinfo, int ref) : tLuaDispatch(L, pTypeinfo, ref) {
    // initialize all our variables -- we decided against using a memory-zeroing
    // memory allocator, so we sort of have to do this work now ...
    //
    m_pClientSite = NULL;
    m_pControlSite = NULL;
    m_pInPlaceSite = NULL;
    m_pInPlaceFrame = NULL;
    m_pInPlaceUIWindow = NULL;

    // certain hosts don't like 0,0 as your initial size, so we're going to set
    // our initial size to 100,50 [so it's at least sort of visible on the screen]
    //
    lua_getref(L, table_ref);
	lua_pushstring(L, "InitialSize");
	lua_gettable(L, -2);
	lua_pushvalue(L, -2);
	lua_remove(L, -3);
	const char * err;
	if(!luaCompat_call(L, 1, 2, &err)) {
		m_Size.cx = (LONG)lua_tointeger(L,-2);
		m_Size.cy = (LONG)lua_tointeger(L,-1);
	} else {
		m_Size.cx = 50;
		m_Size.cy = 20;
	}
    memset(&m_rcLocation, 0, sizeof(m_rcLocation));

    m_hRgn = NULL;
    m_hwnd = NULL;
    m_hwndParent = NULL;

    m_pSimpleFrameSite = NULL;
    m_pOleAdviseHolder = NULL;
    m_pViewAdviseSink = NULL;
    m_pDispAmbient = NULL;

    m_fDirty = FALSE;
    m_fInPlaceActive = FALSE;
    m_fInPlaceVisible = FALSE;
    m_fUIActive = FALSE;
}

tLuaControl::~tLuaControl() {
    // if we've still got a window, go and kill it now.
    //
    if (m_hwnd) {
        // so our window proc doesn't crash.
        //
        DestroyWindow();
    }

    if (m_hRgn != NULL)
    {
       DeleteObject(m_hRgn);
       m_hRgn = NULL;
    }

    // clean up all the pointers we're holding around.
    //
	QUICK_RELEASE(m_pClientSite);
    QUICK_RELEASE(m_pControlSite);
    QUICK_RELEASE(m_pInPlaceSite);
    QUICK_RELEASE(m_pInPlaceFrame);
    QUICK_RELEASE(m_pInPlaceUIWindow);
    QUICK_RELEASE(m_pSimpleFrameSite);
    QUICK_RELEASE(m_pOleAdviseHolder);
    QUICK_RELEASE(m_pViewAdviseSink);
    QUICK_RELEASE(m_pDispAmbient);
}

void tLuaControl::DestroyWindow() {
    lua_getref(L, table_ref);
	lua_pushstring(L, "DestroyWindow");
	lua_gettable(L, -2);
	lua_pushvalue(L, -2);
	lua_remove(L, -3);
	luaCompat_call(L, 1, 0, NULL);
}

STDMETHODIMP tLuaControl::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(IsEqualIID(riid, IID_IUnknown)  ||
       IsEqualIID(riid, IID_IDispatch) ||
       IsEqualIID(riid, interface_iid)) {
      *ppv = (IDispatch*)this;
	  this->AddRef();
      return NOERROR;       
    }

    if((IsEqualIID(riid, IID_IProvideClassInfo) ||
        IsEqualIID(riid, IID_IProvideClassInfo2)   )  &&
       classinfo2)
    {
      *ppv = classinfo2;
      classinfo2->AddRef();
      return NOERROR;
    }

    if(IsEqualIID(riid, IID_IConnectionPointContainer) &&
      cpc)
    {
      *ppv = cpc;
      cpc->AddRef();
      return NOERROR;
    }

    if(IsEqualIID(riid, IID_ILuaDispatch))
    {
      *ppv = (ILuaDispatch*)this;
      this->AddRef();
      return NOERROR;
    }

    if(IsEqualIID(riid, IID_IOleObject))
    {
      *ppv = (IOleObject*)this;
      this->AddRef();
      return NOERROR;
    }

    if(IsEqualIID(riid, IID_IOleControl))
    {
      *ppv = (IOleControl*)this;
      this->AddRef();
      return NOERROR;
    }

    if(IsEqualIID(riid, IID_IOleInPlaceObject))
    {
      *ppv = (IOleInPlaceObject*)this;
      this->AddRef();
      return NOERROR;
    }

    if(IsEqualIID(riid, IID_IOleInPlaceActiveObject))
    {
      *ppv = (IOleInPlaceActiveObject*)this;
      this->AddRef();
      return NOERROR;
    }

    if(IsEqualIID(riid, IID_IViewObject))
    {
      *ppv = (IViewObject2*)this;
      this->AddRef();
      return NOERROR;
    }

    if(IsEqualIID(riid, IID_IViewObject2))
    {
      *ppv = (IViewObject2*)this;
      this->AddRef();
      return NOERROR;
    }

/*    if(IsEqualIID(riid, IID_IQuickActivate))
    {
      *ppv = (IQuickActivate*)this;
      this->AddRef();
      return NOERROR;
    }*/

    if(IsEqualIID(riid, IID_IPersistStreamInit))
    {
      *ppv = (IPersistStreamInit*)this;
      this->AddRef();
      return NOERROR;
    }

    if(IsEqualIID(riid, IID_IPersistStorage))
    {
      *ppv = (IPersistStorage*)this;
      this->AddRef();
      return NOERROR;
    }

    *ppv = NULL;
    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) tLuaControl::AddRef()
{
	return ++m_refs;
}


STDMETHODIMP_(ULONG) tLuaControl::Release()
{
  assert(m_refs > 0);
  if(--m_refs == 0)
  {
    // destrava tabela LUA
    lua_unref(L, table_ref);

    // libera libs

    while(num_methods--)
    {
      typeinfo->ReleaseFuncDesc(funcinfo[num_methods].funcdesc);
      delete funcinfo[num_methods].name;
    }

    delete funcinfo;
    typeinfo->Release();
    typeinfo = NULL;

    delete typehandler;

    if(cpc)
      delete cpc;

    if(classinfo2)
      delete classinfo2;

    // destroi objeto
    delete this;
    return 0;
  }

  return m_refs;
}

//=--------------------------------------------------------------------------=
// COleControl::GetControlInfo    (IOleControl)
//=--------------------------------------------------------------------------=
// returns some information on a control, such as an accelerator table, and
// flags.  really used for keyboard handling and mnemonics
//
// Parameters:
//    CONTROLINFO *        - [in]  where to put said information
//
// Output:
//    HRESULT              - S_OK
//
// Notes:
//
STDMETHODIMP tLuaControl::GetControlInfo(CONTROLINFO *pControlInfo)
{
    CHECK_POINTER(pControlInfo);

    // certain hosts have a bug in which it doesn't initialize the cb in the
    // CONTROLINFO structure, so we can only assert on that here.
    //
    ASSERT(pControlInfo->cb == sizeof(CONTROLINFO), "Host doesn't initialize CONTROLINFO structure");

    // NOTE: control writers should override this routine if they want to
    // return accelerator information in their control.
    //
    pControlInfo->hAccel = NULL;
    pControlInfo->cAccel = 0;

    return S_OK;
}

//=--------------------------------------------------------------------------=
// COleControl::OnMnemonic    [IOleControl]
//=--------------------------------------------------------------------------=
// the container has decided to pass on a key that the end-user has pressed to
// us.  default implementation will be to just activate the control.  people
// looking for more functionality should override this method.
//
// Parameters:
//    LPMSG                - [in] message for this mnemonic
//
// Output:
//    HRESULT              - S_OK, E_POINTER
//
// Notes:
//
STDMETHODIMP tLuaControl::OnMnemonic(LPMSG pMsg)
{
    // OVERRIDE: default implementation is to just activate our control.  
    // user can override if they want more interesting behaviour.
    //
    return InPlaceActivate(TRUE);
}

//=--------------------------------------------------------------------------=
// COleControl:OnAmbientPropertyChange    [IOleControl]
//=--------------------------------------------------------------------------=
// a container calls this whenever it changes an ambient property.
//
// Parameters:
//    DISPID            - [in] dispid of the property that changed.
//
// Output:
//    HRESULT           - S_OK
//
// Notes:
//
STDMETHODIMP tLuaControl::OnAmbientPropertyChange(DISPID dispid)
{
    return S_OK;
}

//=--------------------------------------------------------------------------=
// COleControl::FreezeEvents    [IOleControl]
//=--------------------------------------------------------------------------=
// allows a container to freeze all of a controls events.  when events are
// frozen, a control will not fire any of them.
//
// Parameters:
//    BOOL            - [in] TRUE means FREEZE, FALSE means THAW
//
// Output:
//    HRESULT         - S_OK
//
STDMETHODIMP tLuaControl::FreezeEvents(BOOL fFreeze)
{
    return S_OK;
}

//=--------------------------------------------------------------------------=
// COleControl::SetClientSite    [IOleObject]
//=--------------------------------------------------------------------------=
// informs the embedded object [control] of it's client site [display
// location] within it's container
//
// Parameters:
//    IOleClientSite *        - [in] pointer to client site.
//
// Output:
//    HRESULT                 - S_OK, E_UNEXPECTED
//
// Notes:
//
STDMETHODIMP tLuaControl::SetClientSite(IOleClientSite* pcs) {
    RELEASE_OBJECT(m_pClientSite);
    RELEASE_OBJECT(m_pControlSite);
    RELEASE_OBJECT(m_pSimpleFrameSite);

    // store away the new client site
    //
    m_pClientSite = pcs;

    // if we've actually got one, then get some other interfaces we want to keep
    // around, and keep a handle on it
    //
    if (m_pClientSite) {
        m_pClientSite->AddRef();
        m_pClientSite->QueryInterface(IID_IOleControlSite, (void **)&m_pControlSite);
    }

    return S_OK;
}

//=--------------------------------------------------------------------------=
// COleControl::GetClientSite    [IOleObject]
//=--------------------------------------------------------------------------=
// obtains a pointer to the controls client site.
//
// Parameters:
//    IOleClientSite **        - [out]
//
// Output:
//    HRESULT                  - S_OK
//
// Notes:
//
STDMETHODIMP tLuaControl::GetClientSite(IOleClientSite **ppClientSite)
{
    CHECK_POINTER(ppClientSite);

    *ppClientSite = m_pClientSite;
    ADDREF_OBJECT(*ppClientSite);
    return S_OK;
}

//=--------------------------------------------------------------------------=
// COleControl::SetHostNames    [IOleObject]
//=--------------------------------------------------------------------------=
// Provides the control with the name of its container application and the
// compound document in which it is embedded
//
// Parameters:
//    LPCOLESTR        - [in] name of container application
//    LPCOLESTR        - [in] name of container document
//
// Output:
//    HRESULT          - S_OK
//
// Notes:
//    - we don't care about this
//
STDMETHODIMP tLuaControl::SetHostNames(LPCOLESTR szContainerApp, LPCOLESTR szContainerObj) {
	return S_OK;
}

//=--------------------------------------------------------------------------=
// COleControl::Close    [IOleObject]
//=--------------------------------------------------------------------------=
// Changes the control from the running to the loaded state
//
// Parameters:
//    DWORD             - [in] indicates whether to save the object before closing
//
// Output:
//    HRESULT           - S_OK, OLE_E_PROMPTSAVECANCELLED
//
// Notes:
//
STDMETHODIMP tLuaControl::Close(DWORD dwSaveOption)
{
    HRESULT hr;

    if (m_fInPlaceActive) {
        hr = InPlaceDeactivate();
        RETURN_ON_FAILURE(hr);
    }

    // handle the save flag.
    //
    if ((dwSaveOption == OLECLOSE_SAVEIFDIRTY || dwSaveOption == OLECLOSE_PROMPTSAVE) && m_fDirty) {
        if (m_pOleAdviseHolder) m_pOleAdviseHolder->SendOnSave();
    }
    return S_OK;
}

//=--------------------------------------------------------------------------=
// COleControl::SetMoniker    [IOleObject]
//=--------------------------------------------------------------------------=
// Notifies an object of its container's moniker, the object's own moniker
// relative to the container, or the object's full moniker
//
// Parameters:
//    DWORD                - [in] which moniker is being set
//    IMoniker *           - [in] the moniker
//
// Output:
//    HRESULT              - S_OK, E_FAIL
//
// Notes:
//    - we don't support monikers.
//
STDMETHODIMP tLuaControl::SetMoniker(DWORD dwWitchMoniker, IMoniker* pmk) {
	return E_NOTIMPL;
}

//=--------------------------------------------------------------------------=
// COleControl::GetMoniker    [IOleObject]
//=--------------------------------------------------------------------------=
// Returns a embedded object's moniker, which the caller can use to link to
// the object
//
// Parameters:
//    DWORD            - [in]  how it's assigned
//    DWORD            - [in]  which moniker
//    IMoniker **      - [out] duh.
//
// Output:
//    HRESULT          - E_NOTIMPL
//
// Notes:
//    - we don't support monikers
//
STDMETHODIMP tLuaControl::GetMoniker(DWORD dwAssign, DWORD dwWitchMoniker, IMoniker** ppMoniker) {
	return E_NOTIMPL;
}

//=--------------------------------------------------------------------------=
// COleControl::InitFromData    [IOleObject]
//=--------------------------------------------------------------------------=
// Initializes a newly created object with data from a specified data object,
// which can reside either in the same container or on the Clipboard
//
// Parameters:
//    IDataObject*    - [in] data object with the data
//    BOOL            - [in] how object is created
//    DWORD           - reserved
//
// Output:
//    HRESULT         - S_OK, S_FALSE, E_NOTIMPL, OLE_E_NOTRUNNING
//
// Notes:
//    - we don't have data object support
//
STDMETHODIMP tLuaControl::InitFromData(IDataObject* pDataObject, BOOL fCreation, DWORD dwReserved) {
	return E_NOTIMPL;
}

//=--------------------------------------------------------------------------=
// COleControl::IsUpToDate    [IOleObject]
//=--------------------------------------------------------------------------=
// Checks recursively whether or not an object is up to date.
//
// Output:
//    HRESULT        - S_OK, S_FALSE, OLE_E_UNVAILABLE
//
// Notes:
//
STDMETHODIMP tLuaControl::IsUpToDate() {
	return S_OK;
}

//=--------------------------------------------------------------------------=
// COleControl::GetClipboardData    [IOleObject]
//=--------------------------------------------------------------------------=
// Retrieves a data object containing the current contents of the control.
// Using the pointer to this data object, it is possible to create a new control
// with the same data as the original
//
// Parameters:
//    DWORD          - reserved
//    IDataObject ** - [out] data object for this control
//
// Output:
//    HREUSLT        - S_OK, E_NOTIMPL, OLE_E_NOTRUNNING
//
// Notes:
//
STDMETHODIMP tLuaControl::GetClipboardData(DWORD dwReserved, IDataObject** ppDataObject) {
	*ppDataObject = NULL;
	return E_NOTIMPL;
}

//=--------------------------------------------------------------------------=
// COleControl::DoVerb    [IOleObject]
//=--------------------------------------------------------------------------=
// Requests an object to perform an action in response to an end-user's
// action.
//
// Parameters:
//    LONG             - [in]  verb to be performed
//    LPMSG            - [in]  event that invoked the verb
//    IOleClientSite * - [in]  the controls active client site
//    LONG             - [in]  reserved
//    HWND             - [in]  handle of window containing the object.
//    LPCRECT          - [in]  pointer to objects's display rectangle
//
// Output:
//    HRESULT          - S_OK, OLE_E_NOTINPLACEACTIVE, OLE_E_CANT_BINDTOSOURCE,
//                       DV_E_LINK, OLEOBJ_S_CANNOT_DOVERB_NOW, OLEOBJ_S_INVALIDHWND,
//                       OLEOBJ_E_NOVERBS, OLEOBJ_S_INVALIDVERB, MK_E_CONNECT,
//                       OLE_CLASSDIFF, E_NOTIMPL
//
// Notes:
//
STDMETHODIMP tLuaControl::DoVerb(LONG lVerb, LPMSG pMsg, IOleClientSite *pActiveSite,
    LONG lIndex, HWND hwndParent, LPCRECT prcPosRect)
{
    HRESULT hr;

    switch (lVerb) {
      case OLEIVERB_SHOW:
      case OLEIVERB_INPLACEACTIVATE:
      case OLEIVERB_UIACTIVATE:
        hr = InPlaceActivate(lVerb==OLEIVERB_UIACTIVATE);
        return (hr);

      case OLEIVERB_HIDE:
        UIDeactivate();
        if (m_fInPlaceVisible) SetInPlaceVisible(FALSE);
        return S_OK;

      default:
        // if it's a derived-control defined verb, pass it on to them
        //
        if (lVerb > 0) {

            // FIX-TODO!!! hr is undefined here.
            // http://lua-users.org/lists/lua-l/2006-09/msg00030.html
            #pragma message("FIX - luacom bug http://lua-users.org/lists/lua-l/2006-09/msg00030.html")
            FAIL("FIX - luacom bug lua-l/2006-09/msg00030.html");

            if (hr == OLEOBJ_S_INVALIDVERB) {
                // unrecognised verb -- just do the primary verb and
                // activate the sucker.
                //
                hr = InPlaceActivate(0);
                return (FAILED(hr)) ? hr : OLEOBJ_S_INVALIDVERB;
            } else
                return hr;
        } else {
            FAIL("Unrecognized Negative verb in DoVerb().  bad.");
            return E_NOTIMPL;
        }
        break;
    }

    // dead code
    FAIL("this should be dead code!");
}

//=--------------------------------------------------------------------------=
// COleControl::EnumVerbs    [IOleObject]
//=--------------------------------------------------------------------------=
// create an enumerator object for the verbs this object supports.
//
// Parameters:
//    IEnumOleVERB **    - [out] new enumerator.
//
// Output:
//    HRESULT            - S_OK, E_OUTOFMEMORY
//
// Notes:
//
STDMETHODIMP tLuaControl::EnumVerbs(IEnumOLEVERB** ppEnumVerbs) { 
    return CEnumOLEVERB::CreateInstance(verbs, 5, ppEnumVerbs);
}

//=--------------------------------------------------------------------------=
// COleControl::Update    [IOleObject]
//=--------------------------------------------------------------------------=
// Updates an object handler's or link object's data or view caches.
//
// Output:
//    HRESULT            - S_OK
//
// Notes:
//
STDMETHODIMP tLuaControl::Update() {
	return S_OK; 
}

//=--------------------------------------------------------------------------=
// COleControl::GetUserClassID    [IOleObject]
//=--------------------------------------------------------------------------=
// Returns the controls class identifier, the CLSID corresponding to the
// string identifying the object to an end user.
//
// Parameters:
//    CLSID *      - [in] where to put the CLSID
//
// Output:
//    HRESULT      - S_OK, E_FAIL
//
// Notes:
//
STDMETHODIMP tLuaControl::GetUserClassID(CLSID* pClsid) {
	if(!pClsid) return E_POINTER;
    lua_getref(L, table_ref);
	lua_pushstring(L, "GetClass");
	lua_gettable(L, -2);
	lua_pushvalue(L, -2);
	lua_remove(L, -3);
	if(luaCompat_call(L, 1, 1, NULL)) return E_FAIL;
	const char * classID = lua_tostring(L, -1);
	return CLSIDFromString(tUtil::string2bstr(classID),pClsid);
}

//=--------------------------------------------------------------------------=
// COleControl::GetUserType    [IOleObject]
//=--------------------------------------------------------------------------=
// Retrieves the user-type name of the control for display in user-interface
// elements such as menus, list boxes, and dialog boxes.
//
// Parameters:
//    DWORD        - [in]  specifies the form of the type name.
//    LPOLESTR *   - [out] where to put user type
//
// Output:
//    HRESULT      - S_OK, OLE_S_USEREG, E_OUTOFMEMORY
//
// Notes:
//
STDMETHODIMP tLuaControl::GetUserType(DWORD dwFormOfType, LPOLESTR* pszUserType) {
	CLSID clsid;
	this->GetUserClassID(&clsid);
	return OleRegGetUserType(clsid, dwFormOfType, pszUserType);
}

//=--------------------------------------------------------------------------=
// COleControl::SetExtent    [IOleObject]
//=--------------------------------------------------------------------------=
// Informs the control of how much display space its container has assigned it.
//
// Parameters:
//    DWORD            - [in] which form or 'aspect'  is to be displayed.
//    SIZEL *          - [in] size limit for the control.
//
// Output:
//    HRESULT          - S_OK, E_FAIL, OLE_E_NOTRUNNING
//
// Notes:
//
STDMETHODIMP tLuaControl::SetExtent(DWORD  dwDrawAspect, SIZEL *psizel)
{
    SIZEL sl;
    RECT  rect;

    if (dwDrawAspect & DVASPECT_CONTENT) {

        // change the units to pixels, and resize the control.
        //
        HimetricToPixel(psizel, &sl);

        // first call the user version.  if they return FALSE, they want
        // to keep their current size
        //
	    lua_getref(L, table_ref);
		lua_pushstring(L, "SetExtent");
		lua_gettable(L, -2);
		lua_pushvalue(L, -2);
		lua_remove(L, -3);
		lua_pushnumber(L, sl.cx);
		lua_pushnumber(L, sl.cy);
	    if(luaCompat_call(L, 3, 1, NULL)) return E_FAIL;
        
		if(luaCompat_toCBool(L,-1))
            HimetricToPixel(psizel, &m_Size);

        // set things up with our HWND
        //
        if (m_fInPlaceActive) {

            // theoretically, one should not need to call OnPosRectChange
            // here, but there appear to be a few host related issues that
            // will make us keep it here.  we won't, however, both with
            // windowless ole controls, since they are all new hosts who
            // should know better
            //
            GetWindowRect(m_hwnd, &rect);
            MapWindowPoints(NULL, m_hwndParent, (LPPOINT)&rect, 2);
            rect.right = rect.left + m_Size.cx;
            rect.bottom = rect.top + m_Size.cy;
            m_pInPlaceSite->OnPosRectChange(&rect);

            if (m_hwnd) {
                // just go and resize
                //
                SetWindowPos(m_hwnd, m_hwndParent, 0, 0, m_Size.cx, m_Size.cy,
                                SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
            }
        } else if (m_hwnd) {
            SetWindowPos(m_hwnd, m_hwndParent, 0, 0, m_Size.cx, m_Size.cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        }

        // return code depending on whether or not user accepted given
        // size
        //
        return (luaCompat_toCBool(L,-1)) ? S_OK : E_FAIL;
    } else {
        // we don't support any other aspects.
        //
        return DV_E_DVASPECT;
    }

    // dead code
    FAIL("This should be dead code");
}

//=--------------------------------------------------------------------------=
// COleControl::GetExtent    [IOleObject]
//=--------------------------------------------------------------------------=
// Retrieves the control's current display size.
//
// Parameters:
//    DWORD            - [in] aspect
//    SIZEL *          - [in] where to put results
//
// Output:
//    S_OK, E_INVALIDARG
//
// Notes:
//
STDMETHODIMP tLuaControl::GetExtent(DWORD  dwDrawAspect, SIZEL *pSizeLOut)
{
    if (dwDrawAspect & DVASPECT_CONTENT) {
        PixelToHimetric((const SIZEL *)&m_Size, pSizeLOut);
        return S_OK;
    } else {
        return DV_E_DVASPECT;
    }
}

//=--------------------------------------------------------------------------=
// COleControl::Advise    [IOleObject]
//=--------------------------------------------------------------------------=
// establishes and advisory connection between the control and the container,
// in which the control will notify the container of certain events.
//
// Parameters:
//    IAdviseSink *     - [in]  advise sink of calling object
//    DWORD             - [out] cookie
//
// Output:
//    HRESULT           - S_OK, E_OUTOFMEMORY
//
// Notes:
//
STDMETHODIMP tLuaControl::Advise(IAdviseSink *pAdviseSink, DWORD *pdwConnection)
{
    HRESULT hr;

    // if we haven't yet created a standard advise holder object, do so
    // now
    //
    if (!m_pOleAdviseHolder) {
        hr = CreateOleAdviseHolder(&m_pOleAdviseHolder);
        RETURN_ON_FAILURE(hr);
    }

    // just get it to do the work for us!
    //
    return m_pOleAdviseHolder->Advise(pAdviseSink, pdwConnection);
}

//=--------------------------------------------------------------------------=
// COleControl::Unadvise    [IOleObject]
//=--------------------------------------------------------------------------=
// Deletes a previously established advisory connection.
//
// Parameters:
//    DWORD         - [in] connection cookie
//
// Output:
//    HRESULT       - S_OK, E_FAIL, OLE_E_NOCONNECTION
//
// Notes:
//
STDMETHODIMP tLuaControl::Unadvise(DWORD dwConnection)
{
    if (!m_pOleAdviseHolder) {
        FAIL("Somebody called Unadvise on IOleObject without calling Advise!");
        return E_FAIL;
    }

    return m_pOleAdviseHolder->Unadvise(dwConnection);
}

//=--------------------------------------------------------------------------=
// COleControl::EnumAdvise    [IOleObject]
//=--------------------------------------------------------------------------=
// Enumerates the advisory connections registered for an object, so a container
// can know what to release prior to closing down.
//
// Parameters:
//    IEnumSTATDATA **    - [out] where to put enumerator
//
// Output:
//    HRESULT             - S_OK, E_FAIL, E_NOTIMPL
//
// Notes:
//
STDMETHODIMP tLuaControl::EnumAdvise(IEnumSTATDATA **ppEnumOut)
{
    if (!m_pOleAdviseHolder) {
        FAIL("Somebody Called EnumAdvise without setting up any connections");
        *ppEnumOut = NULL;
        return E_FAIL;
    }

    return m_pOleAdviseHolder->EnumAdvise(ppEnumOut);
}

//=--------------------------------------------------------------------------=
// COleControl::GetMiscStatus    [IOleObject]
//=--------------------------------------------------------------------------=
// Returns a value indicating the status of an object at creation and loading.
//
// Parameters:
//    DWORD         - [in]  aspect desired
//    DWORD *       - [out] where to put the bits.
//
// Output:
//    HRESULT       - S_OK, OLE_S_USEREG, CO_E_CLASSNOTREG, CO_E_READREGDB
//
// Notes:
//
STDMETHODIMP tLuaControl::GetMiscStatus(DWORD dwAspect, DWORD* pdwStatus) {
	if(!pdwStatus) return E_POINTER;
	if(dwAspect != DVASPECT_CONTENT) return *pdwStatus = 0, E_FAIL;
	*pdwStatus = OLEMISC_RECOMPOSEONRESIZE | OLEMISC_INSIDEOUT | OLEMISC_ACTIVATEWHENVISIBLE
		| OLEMISC_ALWAYSRUN;
	return S_OK;
}

//=--------------------------------------------------------------------------=
// COleControl::SetColorScheme    [IOleObject]
//=--------------------------------------------------------------------------=
// Specifies the color palette that the object application should use when it
// edits the specified object.
//
// Parameters:
//    LOGPALETTE *     - [in] new palette
//
// Output:
//    HRESULT          - S_OK, E_NOTIMPL, OLE_E_PALETTE, OLE_E_NOTRUNNING
//
// Notes:
//    - we don't care.
//
STDMETHODIMP tLuaControl::SetColorScheme(LOGPALETTE* pLogpal) {
	return E_NOTIMPL;
}

//=--------------------------------------------------------------------------=
// COleControl::GetWindow    [IOleWindow/IOleInPlaceObject]
//=--------------------------------------------------------------------------=
// Returns the window handle to one of the windows participating in in-place
// activation (frame, document, parent, or in-place object window).
//
// Parameters:
//    HWND *        - [out] where to return window handle.
//
// Output:
//    HRESULT       - S_OK, E_INVALIDARG, E_OUTOFMEMORY, E_UNEXPECTED, E_FAIL
//
// Notes:
//    - this routine has slightly different semantics for windowless controls
//
STDMETHODIMP tLuaControl::GetWindow(HWND* phwnd) {
    *phwnd = m_hwnd;

    return (*phwnd) ? S_OK : E_UNEXPECTED;
}

//=--------------------------------------------------------------------------=
// COleControl::ContextSensitiveHelp    [IOleWindow/IOleInPlaceObject]
//=--------------------------------------------------------------------------=
// Determines whether context-sensitive help mode should be entered during an
// in-place activation session.
//
// Parameters:
//    BOOL            - [in] whether or not to enter help mode.
//
// Output:
//    HRESULT         - S_OK, E_INVALIDARG, E_OUTOFMEMORY, E_UNEXPECTED
//
// Notes:
//
STDMETHODIMP tLuaControl::ContextSensitiveHelp(BOOL fEnterMode) {
	return E_NOTIMPL;
}

//=--------------------------------------------------------------------------=
// COleControl::InPlaceActivate
//=--------------------------------------------------------------------------=
// activates the control, and depending on the verb, optionally ui activates
// it as well.
//
// Parameters:
//    LONG         - [in] the verb that caused us to activate
//
// Output:
//    HRESULT
//
// Notes:
//    - this is spaghetti code at it's worst.  effectively, we have to
//      be able to handle three types of site pointers -- IOleInPlaceSIte,
//      IOleInPlaceSiteEx, and IOleInPlaceSiteWindowless.  not terribly
//      pretty.
//
HRESULT tLuaControl::InPlaceActivate(BOOL lVerb)
{
    SIZEL sizel;
    IOleInPlaceSiteEx *pIPSEx = NULL;
    HRESULT hr;
    BOOL    fNoRedraw = FALSE;

    // if we don't have a client site, then there's not much to do.
    //
    if (!m_pClientSite)
        return S_OK;

    // get an InPlace site pointer.
    //
    hr = m_pClientSite->QueryInterface(IID_IOleInPlaceSite, (void **)&m_pInPlaceSite);
    RETURN_ON_FAILURE(hr);

    // if we're not already active, go and do it.
    //
    if (!m_fInPlaceActive) {
        OLEINPLACEFRAMEINFO InPlaceFrameInfo;
        RECT rcPos, rcClip;

        hr = m_pInPlaceSite->CanInPlaceActivate();
        if (hr != S_OK) {
            hr = (FAILED(hr)) ? E_FAIL : hr;
            goto CleanUp;
        }

        // if we are here, then we have permission to go in-place active.
        // now, announce our intentions to actually go ahead and do this.
        //
        m_pInPlaceSite->OnInPlaceActivate();
        CLEANUP_ON_FAILURE(hr);

        // if we're here, we're ready to go in-place active.  we just need
        // to set up some flags, and then create the window [if we have
        // one]
        //
        m_fInPlaceActive = TRUE;

        // we need to get some information about our location in the parent
        // window, as well as some information about the parent
        //
        InPlaceFrameInfo.cb = sizeof(OLEINPLACEFRAMEINFO);
        hr = m_pInPlaceSite->GetWindow(&m_hwndParent);
        if (SUCCEEDED(hr))
            hr = m_pInPlaceSite->GetWindowContext(&m_pInPlaceFrame, &m_pInPlaceUIWindow, &rcPos, &rcClip, &InPlaceFrameInfo);
        CLEANUP_ON_FAILURE(hr);

        // make sure we'll display ourselves in the correct location with the correct size
        //
        sizel.cx = rcPos.right - rcPos.left;
        sizel.cy = rcPos.bottom - rcPos.top;
        m_Size = sizel;
        SetObjectRects(&rcPos, &rcClip);

        // finally, create our window if we have to!
        // create the window, and display it.  die horribly if we couldnt'
        //
        if (!CreateInPlaceWindow(rcPos.left, rcPos.top, fNoRedraw)) {
            hr = E_FAIL;
            goto CleanUp;
        }
    }

    // if we're not inplace visible yet, do so now.
    //
    if (!m_fInPlaceVisible)
        SetInPlaceVisible(TRUE);

    // if we weren't asked to UIActivate, then we're done.
    //
    if (!lVerb)
        return S_OK;

    // if we're not already UI active, do sow now.
    //
    if (!m_fUIActive) {
        m_fUIActive = TRUE;

        // inform the container of our intent
        //
        m_pInPlaceSite->OnUIActivate();

        // take the focus  [which is what UI Activation is all about !]
        //
        SetFocus(m_hwnd);

        // set ourselves up in the host.
        //
        m_pInPlaceFrame->SetActiveObject((IOleInPlaceActiveObject *)this, NULL);
        if (m_pInPlaceUIWindow)
            m_pInPlaceUIWindow->SetActiveObject((IOleInPlaceActiveObject *)this, NULL);

        // we have to explicitly say we don't wany any border space.
        //
        m_pInPlaceFrame->SetBorderSpace(NULL);
        if (m_pInPlaceUIWindow)
            m_pInPlaceUIWindow->SetBorderSpace(NULL);
    }

    // be-de-be-de-be-de that's all folks!
    //
    return S_OK;

  CleanUp:
    // something catastrophic happened [or, at least something bad].
    // die a horrible fiery mangled painful death.
    //
    m_fInPlaceActive = FALSE;
    return hr;
}

//=--------------------------------------------------------------------------=
// COleControl::InPlaceDeactivate    [IOleInPlaceObject]
//=--------------------------------------------------------------------------=
// Deactivates an active in-place object and discards the object's undo state.
//
// Output:
//    HRESULT        - S_OK, E_UNEXPECTED
//
// Notes:
//
STDMETHODIMP tLuaControl::InPlaceDeactivate(void)
{
    // if we're not in-place active yet, then this is easy.
    //
    if (!m_fInPlaceActive)
        return S_OK;

    // transition from UIActive back to active
    //
    if (m_fUIActive)
        UIDeactivate();

    m_fInPlaceActive = FALSE;
    m_fInPlaceVisible = FALSE;

    // if we have a window, tell it to go away.
    //
    if (m_hwnd) {
        // so our window proc doesn't crash.
        //
        DestroyWindow();
        m_hwnd = NULL;
    }

    RELEASE_OBJECT(m_pInPlaceFrame);
    RELEASE_OBJECT(m_pInPlaceUIWindow);
    m_pInPlaceSite->OnInPlaceDeactivate();
    return S_OK;
}

//=--------------------------------------------------------------------------=
// COleControl::UIDeactivate    [IOleInPlaceObject]
//=--------------------------------------------------------------------------=
// transitions us from UI Active to merely being active [visible]  for
// a control, this doesn't mean all that much.
//
// Output:
//    HRESULT         - S_OK, E_UNEXPECTED
//
// Notes:
//
STDMETHODIMP tLuaControl::UIDeactivate(void)
{
    // if we're not UIActive, not much to do.
    //
    if (!m_fUIActive)
        return S_OK;

    m_fUIActive = FALSE;

    // notify frame windows, if appropriate, that we're no longer ui-active.
    //
    if (m_pInPlaceUIWindow) m_pInPlaceUIWindow->SetActiveObject(NULL, NULL);
    m_pInPlaceFrame->SetActiveObject(NULL, NULL);

    // we don't need to explicitly release the focus here since somebody
    // else grabbing the focus is what is likely to cause us to get lose it
    //
    m_pInPlaceSite->OnUIDeactivate(FALSE);

    return S_OK;
}

//=--------------------------------------------------------------------------=
// COleControl::SetObjectRects    [IOleInPlaceObject]
//=--------------------------------------------------------------------------=
// Indicates how much of the control is visible.
//
// Parameters:
//    LPCRECT        - [in] position of the control.
//    LPCRECT        - [in] clipping rectangle for the control.
//
// Output:
//    HRESULT        - S_OK, E_INVALIDARG, E_OUTOFMEMORY, E_UNEXPECTED
//
// Notes:
//
STDMETHODIMP tLuaControl::SetObjectRects(LPCRECT prcPos, LPCRECT prcClip)
{
    BOOL fRemoveWindowRgn;

    // move our window to the new location and handle clipping. not applicable
    // for windowless controls, since the container will be responsible for all
    // clipping.
    //
    if (m_hwnd) {
        fRemoveWindowRgn = m_fUsingWindowRgn;
        if (prcClip) {
            // the container wants us to clip, so figure out if we really
            // need to
            //
            RECT rcIXect;
            if ( IntersectRect(&rcIXect, prcPos, prcClip) ) {
                if (!EqualRect(&rcIXect, prcPos)) {
                    OffsetRect(&rcIXect, -(prcPos->left), -(prcPos->top));

                    HRGN tempRgn = CreateRectRgnIndirect(&rcIXect);
                    SetWindowRgn(m_hwnd, tempRgn, TRUE);

                    if (m_hRgn != NULL)
                       DeleteObject(m_hRgn);
                    m_hRgn = tempRgn;

                    m_fUsingWindowRgn = TRUE;
                    fRemoveWindowRgn  = FALSE;
                }
            }
        }

        if (fRemoveWindowRgn) {
            SetWindowRgn(m_hwnd, NULL, TRUE);
            if (m_hRgn != NULL)
            {
               DeleteObject(m_hRgn);
               m_hRgn = NULL;
            }
            m_fUsingWindowRgn = FALSE;
        }

        // set our control's location, but don't change it's size at all
        // [people for whom zooming is important should set that up here]
        //
        int cx, cy;
        cx = prcPos->right - prcPos->left;
        cy = prcPos->bottom - prcPos->top;
        SetWindowPos(m_hwnd, m_hwndParent, prcPos->left, prcPos->top, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);
    }

    // save out our current location.  windowless controls want this more
    // that windowed ones do, but everybody can have it just in case
    //
    m_rcLocation = *prcPos;

    return S_OK;
}

//=--------------------------------------------------------------------------=
// COleControl::ReactivateAndUndo    [IOleInPlaceObject]
//=--------------------------------------------------------------------------=
// Reactivates a previously deactivated object, undoing the last state of the object.
//
// Output:
//    HRESULT        - S_OK, E_NOTUNDOABLE
//
// Notes:
//
STDMETHODIMP tLuaControl::ReactivateAndUndo
(
    void
)
{
    return E_NOTIMPL;
}

//=--------------------------------------------------------------------------=
// COleControl::TranslateAccelerator    [IOleInPlaceActiveObject]
//=--------------------------------------------------------------------------=
// Processes menu accelerator-key messages from the container's message queue.
//
// Parameters:
//    LPMSG            - [in] the message that has the special key in it.
//
// Output:
//    HRESULT          - S_OK, S_FALSE, E_UNEXPECTED
//
// Notes:
//
STDMETHODIMP tLuaControl::TranslateAccelerator(LPMSG pmsg)
{
    // we didn't want it.
    //
    return S_FALSE;
}

//=--------------------------------------------------------------------------=
// COleControl::OnFrameWindowActivate    [IOleInPlaceActiveObject]
//=--------------------------------------------------------------------------=
// Notifies the control when the container's top-level frame window is
// activated or deactivated.
//
// Parameters:
//    BOOL        - [in] state of containers top level window.
//
// Output:
//    HRESULT     - S_OK
//
// Notes:
//
STDMETHODIMP tLuaControl::OnFrameWindowActivate
(
    BOOL fActivate
)
{
    // we're supposed to go UI active in this case
    //
    return InPlaceActivate(TRUE);
}

//=--------------------------------------------------------------------------=
// COleControl::OnDocWindowActivate    [IOleInPlaceActiveObject]
//=--------------------------------------------------------------------------=
// Notifies the active control when the container's document window is
// activated or deactivated.
//
// Parameters:
//    BOOL            - state of mdi child window.
//
// Output:
//    HRESULT         - S_OK
//
// Notes:
//
STDMETHODIMP tLuaControl::OnDocWindowActivate
(
    BOOL fActivate
)
{
    // we're supposed to go UI active in this case
    //
    return InPlaceActivate(TRUE);
}

//=--------------------------------------------------------------------------=
// COleControl::ResizeBorder    [IOleInPlaceActiveObject]
//=--------------------------------------------------------------------------=
// Alerts the control that it needs to resize its border space.
//
// Parameters:
//    LPCRECT               - [in] new outer rectangle for border space
//    IOleInPlaceUIWindow * - [in] the document or frame who's border has changed
//    BOOL                  - [in] true if it was the fram window taht called.
//
// Output:
//    HRESULT               - S_OK
//
// Notes:
//
STDMETHODIMP tLuaControl::ResizeBorder(LPCRECT prcBorder,
										IOleInPlaceUIWindow *pUIWindow,
										BOOL fFrameWindow) {
	return S_OK;
}

//=--------------------------------------------------------------------------=
// COleControl::EnableModeless    [IOleInPlaceActiveObject]
//=--------------------------------------------------------------------------=
// Enables or disables modeless dialog boxes when the container creates or
// destroys a modal dialog box.
//
// Parameters:
//    BOOL            - [in] enable or disable modeless dialogs.
//
// Output:
//    HRESULT         - S_OK
//
// Notes:
//
STDMETHODIMP tLuaControl::EnableModeless(BOOL fEnable) {
	return S_OK;
}

//=--------------------------------------------------------------------------=
// COleControl::Draw    [IViewObject2]
//=--------------------------------------------------------------------------=
// Draws a representation of an object onto the specified device context. 
//
// Parameters:
//    DWORD                - [in] draw aspect
//    LONG                 - [in] part of object to draw [not relevant]
//    void *               - NULL
//    DVTARGETDEVICE *     - [in] specifies the target device
//    HDC                  - [in] information context for target device
//    HDC                  - [in] target device context
//    LPCRECTL             - [in] rectangle in which the object is drawn
//    LPCRECTL             - [in] window extent and origin for metafiles
//    BOOL (*)(DWORD)      - [in] callback for continuing or cancelling drawing
//    DWORD                - [in] parameter to pass to callback.
//
// Output:
//    HRESULT
//
// Notes:
//    - we support the following OCX 96 extensions
//        a. flicker free drawing [multi-pass drawing]
//        b. pvAspect != NULL for optimized DC handling
//        c. prcBounds == NULL for windowless inplace active objects
//
STDMETHODIMP tLuaControl::Draw(DWORD dwDrawAspect, LONG lIndex, void *pvAspect,
    DVTARGETDEVICE *ptd, HDC hicTargetDevice, HDC hdcDraw, LPCRECTL prcBounds,
    LPCRECTL prcWBounds, BOOL (__stdcall *pfnContinue)(ULONG_PTR dwContinue), ULONG_PTR dwContinue)
{
    //unused: HWND hwnd;
    RECT rcClient;
    int  iMapMode;
    POINT ptWOrg, ptVOrg;
    SIZE  sWOrg, sVOrg;

    // support the aspects required for multi-pass drawing
    //
    switch (dwDrawAspect) {
        case DVASPECT_CONTENT:
            break;
        default:
            return DV_E_DVASPECT;
    }

    if (hicTargetDevice)
		return E_NOTIMPL;

    if (!m_hwnd)
        return E_FAIL;

    GetClientRect(m_hwnd, &rcClient);

    // set up the DC for painting.  this code largely taken from the MFC CDK
    // DoSuperClassPaint() fn.  doesn't always get things like command
    // buttons quite right ...
    //
    // NOTE: there is a windows 95 problem in which the font instance manager
    // will leak a bunch of bytes in the global GDI pool whenever you 
    // change your extents and have an active font.  this code gets around
    // this for on-screen cases, but not for printing [which shouldn't be
    // too serious, because you're not often changing your control size and
    // printing rapidly in succession]
    //
    if ((rcClient.right - rcClient.left != prcBounds->right - prcBounds->left)
        && (rcClient.bottom - rcClient.top != prcBounds->bottom - prcBounds->top)) {

        iMapMode = SetMapMode(hdcDraw, MM_ANISOTROPIC);
        SetWindowExtEx(hdcDraw, rcClient.right, rcClient.bottom, &sWOrg);
        SetViewportExtEx(hdcDraw, prcBounds->right - prcBounds->left, prcBounds->bottom - prcBounds->top, &sVOrg);
    }

    SetWindowOrgEx(hdcDraw, 0, 0, &ptWOrg);
    SetViewportOrgEx(hdcDraw, prcBounds->left, prcBounds->top, &ptVOrg);

//	SendMessage(WindowFromDC(hdcDraw), WM_PAINT, NULL, NULL);

	LRESULT wndProc = GetWindowLongPtr(m_hwnd, GWLP_WNDPROC);
	if(!wndProc)
		wndProc = GetWindowLongPtr(m_hwnd, DWLP_DLGPROC);
    CallWindowProc((WNDPROC)wndProc, m_hwnd, WM_PAINT, (WPARAM)hdcDraw, 0);

    return S_OK;
}

//=--------------------------------------------------------------------------=
// COleControl::GetColorSet    [IViewObject2]
//=--------------------------------------------------------------------------=
// Returns the logical palette that the control will use for drawing in its
// IViewObject::Draw method with the corresponding parameters.
//
// Parameters:
//    DWORD                - [in]  how the object is to be represented
//    LONG                 - [in]  part of the object to draw [not relevant]
//    void *               - NULL
//    DVTARGETDEVICE *     - [in]  specifies the target device
//    HDC                  - [in]  information context for the target device
//    LOGPALETTE **        - [out] where to put palette
//
// Output:
//    S_OK                 - Control has a palette, and returned it through the out param.
//    S_FALSE              - Control does not currently have a palette.
//    E_NOTIMPL            - Control will never have a palette so optimize handling of this control.
//
// Notes:
//
STDMETHODIMP tLuaControl::GetColorSet(DWORD dwDrawAspect, LONG lindex,
    void *IgnoreMe, DVTARGETDEVICE *ptd, HDC hicTargetDevice, LOGPALETTE **ppColorSet)
{
    if (dwDrawAspect != DVASPECT_CONTENT)
        return DV_E_DVASPECT;

	return E_NOTIMPL;
}

//=--------------------------------------------------------------------------=
// COleControl::Freeze    [IViewObject2]
//=--------------------------------------------------------------------------=
// Freezes a certain aspect of the object's presentation so that it does not
// change until the IViewObject::Unfreeze method is called.
//
// Parameters:
//    DWORD            - [in] aspect
//    LONG             - [in] part of object to draw
//    void *           - NULL
//    DWORD *          - [out] for Unfreeze
//
// Output:
//    HRESULT
//
// Notes:
//
STDMETHODIMP tLuaControl::Freeze(DWORD dwDrawAspect, LONG lIndex, void *IgnoreMe, DWORD *pdwFreeze)
{
    return E_NOTIMPL;
}

//=--------------------------------------------------------------------------=
// COleControl::Unfreeze    [IVewObject2]
//=--------------------------------------------------------------------------=
// Releases a previously frozen drawing. The most common use of this method
// is for banded printing.
//
// Parameters:
//    DWORD        - [in] cookie from freeze
//
// Output:
//    HRESULT
//
// Notes:
//
STDMETHODIMP tLuaControl::Unfreeze(DWORD dwFreeze)
{
    return E_NOTIMPL;
}

//=--------------------------------------------------------------------------=
// COleControl::SetAdvise    [IViewObject2]
//=--------------------------------------------------------------------------=
// Sets up a connection between the control and an advise sink so that the
// advise sink can be notified about changes in the control's view.
//
// Parameters:
//    DWORD            - [in] aspect
//    DWORD            - [in] info about the sink
//    IAdviseSink *    - [in] the sink
//
// Output:
//    HRESULT
//
// Notes:
//
STDMETHODIMP tLuaControl::SetAdvise(DWORD dwAspects, DWORD dwAdviseFlags, IAdviseSink *pAdviseSink)
{
    // if it's not a content aspect, we don't support it.
    //
    if (!(dwAspects & DVASPECT_CONTENT)) {
        return DV_E_DVASPECT;
    }

    // set up some flags  [we gotta stash for GetAdvise ...]
    //
    m_fViewAdvisePrimeFirst = (dwAdviseFlags & ADVF_PRIMEFIRST) ? TRUE : FALSE;
    m_fViewAdviseOnlyOnce = (dwAdviseFlags & ADVF_ONLYONCE) ? TRUE : FALSE;

    RELEASE_OBJECT(m_pViewAdviseSink);
    m_pViewAdviseSink = pAdviseSink;
    ADDREF_OBJECT(m_pViewAdviseSink);

    return S_OK;
}

//=--------------------------------------------------------------------------=
// COleControl::GetAdvise    [IViewObject2]
//=--------------------------------------------------------------------------=
// Retrieves the existing advisory connection on the control if there is one.
// This method simply returns the parameters used in the most recent call to
// the IViewObject::SetAdvise method.
//
// Parameters:
//    DWORD *            - [out]  aspects
//    DWORD *            - [out]  advise flags
//    IAdviseSink **     - [out]  the sink
//
// Output:
//    HRESULT
//
// Notes;
//
STDMETHODIMP tLuaControl::GetAdvise(DWORD *pdwAspects, DWORD *pdwAdviseFlags, IAdviseSink **ppAdviseSink)
{
    // if they want it, give it to them
    //
    if (pdwAspects)
        *pdwAspects = DVASPECT_CONTENT;

    if (pdwAdviseFlags) {
        *pdwAdviseFlags = 0;
        if (m_fViewAdviseOnlyOnce) *pdwAdviseFlags |= ADVF_ONLYONCE;
        if (m_fViewAdvisePrimeFirst) *pdwAdviseFlags |= ADVF_PRIMEFIRST;
    }

    if (ppAdviseSink) {
        *ppAdviseSink = m_pViewAdviseSink;
        ADDREF_OBJECT(*ppAdviseSink);
    }

    return S_OK;
}

//=--------------------------------------------------------------------------=
// COleControl::GetExtent    [IViewObject2]
//=--------------------------------------------------------------------------=
// Returns the size that the control will be drawn on the
// specified target device.
//
// Parameters:
//    DWORD            - [in] draw aspect
//    LONG             - [in] part of object to draw
//    DVTARGETDEVICE * - [in] information about target device
//    LPSIZEL          - [out] where to put the size
//
// Output:
//    HRESULT
//
// Notes:
//
STDMETHODIMP tLuaControl::GetExtent(DWORD dwDrawAspect, LONG lindex, DVTARGETDEVICE *ptd, LPSIZEL psizel)
{
    // we already have an implementation of this [from IOleObject]
    //
    return GetExtent(dwDrawAspect, psizel);
}


//=--------------------------------------------------------------------------=
// COleControl::SetInPlaceVisible    [helper]
//=--------------------------------------------------------------------------=
// controls the visibility of the control window.
//
// Parameters:
//    BOOL        - TRUE shows FALSE hides.
//
// Notes:
//
void tLuaControl::SetInPlaceVisible(BOOL fShow)
{
    BOOL fVisible;

    m_fInPlaceVisible = fShow;

    // don't do anything if we don't have a window.  otherwise, set it
    //
    if (m_hwnd) {
        fVisible = ((GetWindowLong(m_hwnd, GWL_STYLE) & WS_VISIBLE) != 0);

        if (fVisible && !fShow)
            ShowWindow(m_hwnd, SW_HIDE);
        else if (!fVisible && fShow)
            ShowWindow(m_hwnd, SW_SHOWNA);
    }
}

//=--------------------------------------------------------------------------=
// COleControl::CreateInPlaceWindow
//=--------------------------------------------------------------------------=
// creates the window with which we will be working.
// yay.
//
// Parameters:
//    int            - [in] left
//    int            - [in] top
//    BOOL           - [in] can we skip redrawing?
//
// Output:
//    HWND
//
// Notes:
//    - DANGER! DANGER!  this function is protected so that anybody can call it
//      from their control.  however, people should be extremely careful of when
//      and why they do this.  preferably, this function would only need to be
//      called by an end-control writer in design mode to take care of some
//      hosting/painting issues.  otherwise, the framework should be left to
//      call it when it wants.
//
HWND tLuaControl::CreateInPlaceWindow(int x, int y, BOOL fNoRedraw)
{
    BOOL    fVisible;

	// if we've already got a window, do nothing.
    //
    if (m_hwnd)
        return m_hwnd;

    // create window visible if parent hidden (common case)
    // otherwise, create hidden, then shown.  this is a little subtle, but
    // it makes sense eventually.
    //
    if (!m_hwndParent)
        m_hwndParent = GetParkingWindow();

    fVisible = IsWindowVisible(m_hwndParent);

    // we have to mutex the entire create window process since we need to use
    // the s_pLastControlCreated to pass in the object pointer.  nothing too
    // serious
    //
    m_fCreatingWindow = TRUE;

    lua_getref(L, table_ref);
	lua_pushstring(L, "CreateWindow");
	lua_gettable(L, -2);
	lua_pushvalue(L, -2);
	lua_remove(L, -3);
	luaCompat_pushPointer(L, m_hwndParent);
	lua_pushnumber(L, x);
	lua_pushnumber(L, y);
	lua_pushnumber(L, m_Size.cx);
	lua_pushnumber(L, m_Size.cy);
	const char *err;
	int res = luaCompat_call(L, 6, 1, &err);
    if(res) return NULL;

	m_hwnd = (HWND)luaCompat_getPointer(L, -1);
	lua_pop(L, 1);

    // clean up some variables, and leave the critical section
    //
    m_fCreatingWindow = FALSE;

    if (m_hwnd) {
        // if we didn't create the window visible, show it now.
        //
		
        if (fVisible)
		{
			SetWindowPos(m_hwnd, m_hwndParent, x, y, m_Size.cx, m_Size.cy,
						 SWP_NOZORDER | SWP_SHOWWINDOW | ((fNoRedraw) ? SWP_NOREDRAW : 0));
		}
    }

    // finally, tell the host of this
    //
    if (m_pClientSite)
        m_pClientSite->ShowObject();

    return m_hwnd;
}

//=--------------------------------------------------------------------------=
// COleControl::GetClassID    [IPersistStreamInit]
//=--------------------------------------------------------------------------=
// returns the classid of this mamma
//
// Parameters:
//    CLSID *         - [out] where to put the clsid
//
// Output:
//    HRESULT
//
// Notes:
//
STDMETHODIMP tLuaControl::GetClassID(CLSID *pclsid)
{
	return GetUserClassID(pclsid);
}


//=--------------------------------------------------------------------------=
// COleControl::IsDirty    [IPersistStreamInit]
//=--------------------------------------------------------------------------=
// asks if we're dirty or not.  duh.
//
// Output:
//    HRESULT        - S_OK: dirty, S_FALSE: not dirty
//
// Notes:
//
STDMETHODIMP tLuaControl::IsDirty(void)
{
    return S_FALSE;
}

//=--------------------------------------------------------------------------=
// COleControl::InitNew    [IPersistStreamInit]
//=--------------------------------------------------------------------------=
// causes the control to intialize itself with a new bunch of state information
//
// Output:
//    HRESULT
//
// Notes:
//
STDMETHODIMP tLuaControl::InitNew(void)
{
    return S_OK;
}

//=--------------------------------------------------------------------------=
// COleControl::GetSizeMax    [IPersistStreamInit]
//=--------------------------------------------------------------------------=
//
// Parameters:
//    ULARGE_INTEGER *    - [out]
//
// Output:
//    HRESULT
//
// Notes:
//
STDMETHODIMP tLuaControl::GetSizeMax(ULARGE_INTEGER *pulMaxSize)
{
    return E_NOTIMPL;
}

//=--------------------------------------------------------------------------=
// COleControl::Load    [IPersistStreamInit]
//=--------------------------------------------------------------------------=
// load from an IStream
//
// Parameters:
//    IStream *    - [in] stream from which to load
//
// Output:
//    HRESULT
//
// Notes:
//
STDMETHODIMP tLuaControl::Load(IStream *pStream)
{
	return S_OK;
}

//=--------------------------------------------------------------------------=
// COleControl::Save    [IPersistStreamInit]
//=--------------------------------------------------------------------------=
// saves out our state using streams
//
// Parameters:
//    IStream *        - [in]
//    BOOL             - [in] clear dirty bit?
//
// Output:
//    HRESULT
//
// Notes:
//
STDMETHODIMP tLuaControl::Save(IStream *pStream, BOOL fClearDirty)
{
    return S_OK;
}

//=--------------------------------------------------------------------------=
// COleControl::InitNew    [IPersistStorage]
//=--------------------------------------------------------------------------=
// ipersiststorage version of this.  fweee
//
// Parameters:
//    IStorage *    - [in] we don't use this
//
// Output:
//    HRESULT
//
// Notes:
//
STDMETHODIMP tLuaControl::InitNew(IStorage *pStorage)
{
    return InitNew();
}

//=--------------------------------------------------------------------------=
// COleControl::Load    [IPersistStorage]
//=--------------------------------------------------------------------------=
// Ipersiststorage version of this
//
// Parameters:
//    IStorage *    - [in] DUH.
//
// Output:
//    HRESULT
//
// Notes:
//
STDMETHODIMP tLuaControl::Load(IStorage *pStorage)
{
	return S_OK;
}

//=--------------------------------------------------------------------------=
// COleControl::Save    [IPersistStorage]
//=--------------------------------------------------------------------------=
// save into the contents stream of the given storage object.
//
// Parameters:
//    IStorage *        - [in] 10 points if you figure it out
//    BOOL              - [in] is the storage the same as the load storage?
//
// Output:
//    HRESULT
//
// Notes:
//
STDMETHODIMP tLuaControl::Save(IStorage *pStorage, BOOL fSameAsLoad)
{
	return S_OK;
}

//=--------------------------------------------------------------------------=
// COleControl::SaveCompleted    [IPersistStorage]
//=--------------------------------------------------------------------------=
// lets us clear out our flags.
//
// Parameters:
//    IStorage *    - ignored
//
// Output:
//    HRESULT
//
// Notes:
//
STDMETHODIMP tLuaControl::SaveCompleted(IStorage *pStorageNew)
{
    // if our save succeeded, then we can do our post save work.
    //
    /*if (m_fSaveSucceeded) {
        m_fDirty = FALSE;
        if (m_pOleAdviseHolder)
            m_pOleAdviseHolder->SendOnSave();
    }*/

    return S_OK;
}

//=--------------------------------------------------------------------------=
// COleControl::HandsOffStorage    [IPersistStorage]
//=--------------------------------------------------------------------------=
// not interesting
//
// Output:
//    S_OK
//
// Notes:
//
STDMETHODIMP tLuaControl::HandsOffStorage(void)
{
    // we don't ever hold on to  a storage pointer, so this is remarkably
    // uninteresting to us.
    //
    return S_OK;
}

//=--------------------------------------------------------------------------=
// COleControl::QuickActivate    [IQuickActivate]
//=--------------------------------------------------------------------------=
// allows the container to activate the control.
//
// Parameters:
//    QACONTAINER *        - [in]  info about the container
//    QACONTROL *          - [out] info about the control
//
// Output:
//    HRESULT
//
// Notes:
//
STDMETHODIMP tLuaControl::QuickActivate(QACONTAINER *pContainer, QACONTROL *pControl)
{
    HRESULT hr;
    DWORD   dw;

    // we need these guys.
    //
    if (!pContainer) return E_UNEXPECTED;
    if (!pControl) return E_UNEXPECTED;

    // start grabbing things from the QACONTAINER structure and apply them
    // as relevant
    //
    if (pContainer->cbSize < sizeof(QACONTAINER)) return E_UNEXPECTED;
    if (pControl->cbSize < sizeof(QACONTROL)) return E_UNEXPECTED;

    // save out the client site, of course.
    //
    if (pContainer->pClientSite) {
        hr = SetClientSite(pContainer->pClientSite);
        RETURN_ON_FAILURE(hr);
    }

    // then the event sink.
    //
    if (pContainer->pUnkEventSink) {
  	    tLuaCOMConnPointContainer* cpc = GetConnPointContainer();
        IConnectionPoint *connection_point;
		hr = cpc->FindConnectionPoint(IID_IUnknown, &connection_point);
        if (FAILED(hr)) {
            pContainer->pUnkEventSink->Release();
            return hr;
        }
        hr = connection_point->Advise(pContainer->pUnkEventSink, &pControl->dwEventCookie);
        if (FAILED(hr)) {
            pContainer->pUnkEventSink->Release();
            return hr;
        }
    }

    // finally, the advise sink.
    //
    if (pContainer->pAdviseSink) {
        // don't need to pass the cookie back since there can only be one
        // person advising at a time.
        //
        hr = Advise((IAdviseSink*)pContainer->pAdviseSink, &dw);
        RETURN_ON_FAILURE(hr);
    }

    // set up a few things in the QACONTROL structure.  we're opaque by default
    //
    pControl->dwMiscStatus = OLEMISC_RECOMPOSEONRESIZE | OLEMISC_INSIDEOUT | OLEMISC_ACTIVATEWHENVISIBLE | OLEMISC_ALWAYSRUN;
    pControl->dwViewStatus = VIEWSTATUS_OPAQUE;
    pControl->dwPointerActivationPolicy = POINTERINACTIVE_ACTIVATEONENTRY;

    // that's pretty much all we're interested in.  we will, however, pass on the
    // rest of the things to the end control writer and see if they want to do
    // anything with them. they shouldn't touch any of the above except for the
    // ambients.
    //
    return S_OK;
}

//=--------------------------------------------------------------------------=
// COleControl::SetContentExtent    [IQuickActivate]
//=--------------------------------------------------------------------------=
// the container calls this to set the content extent of the control.
//
// Parameters:
//    LPSIZEL            - [in] the size of the content extent
//
// Output:
//    HRESULT            - S_OK, or E_FAIL for fixed size control
//
// Notes:
//
STDMETHODIMP tLuaControl::SetContentExtent(LPSIZEL pSize)
{
    return SetExtent(DVASPECT_CONTENT, pSize);
}

//=--------------------------------------------------------------------------=
// COleControl::GetContentExtent    [IQuickActivate]
//=--------------------------------------------------------------------------=
// the container calls this to get the content extent of the control
//
// Parameters:
//    LPSIZEL        - [out] returns current size
//
// Output:
//    HRESULT
//
// Notes:
//
STDMETHODIMP tLuaControl::GetContentExtent(LPSIZEL pSize)
{
    return GetExtent(DVASPECT_CONTENT, pSize);
}


tLuaControl *tLuaControl::CreateLuaControl(lua_State* L,
                                               ITypeInfo* interface_typeinfo,
                                               int ref
                                               )
{
  tLuaControl *pcont = 
    new tLuaControl(L, interface_typeinfo, ref);

  lua_getref(L, ref);
  luaCompat_pushPointer(L, idxDispatch);
  luaCompat_pushPointer(L, pcont);
  lua_rawset(L,-3);
  lua_pop(L, 1);

  pcont->AddRef();

  return pcont;
}
