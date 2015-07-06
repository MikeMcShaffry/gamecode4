/*
 * tLuaControl.h
 */

#ifndef __TLUACONTROL_H
#define __TLUACONTROL_H

#include <ole2.h>
#include <oleidl.h>
#include <ocidl.h>

#include "tLuaDispatch.h"

class tLuaControl: public tLuaDispatch, public IOleObject,
	public IOleControl,	public IOleInPlaceObject,
	public IOleInPlaceActiveObject, public IViewObject2,
	public IPersistStreamInit, public IPersistStorage, public IQuickActivate
{
public:
    tLuaControl(lua_State* L, ITypeInfo *pTypeinfo, int ref);
    ~tLuaControl();
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    //=--------------------------------------------------------------------------=
    // IPersist methods.  used by IPersistStream and IPersistStorage
    //
    STDMETHOD(GetClassID)(THIS_ LPCLSID lpClassID);

    // IPersistStreamInit methods
    //
    STDMETHOD(IsDirty)(THIS);
    STDMETHOD(Load)(LPSTREAM pStm);
    STDMETHOD(Save)(LPSTREAM pStm, BOOL fClearDirty);
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER FAR* pcbSize);
    STDMETHOD(InitNew)();

    // IPersistStorage
    //
    STDMETHOD(InitNew)(IStorage  *pStg);
    STDMETHOD(Load)(IStorage  *pStg);
    STDMETHOD(Save)(IStorage  *pStgSave, BOOL fSameAsLoad);
    STDMETHOD(SaveCompleted)(IStorage  *pStgNew);
    STDMETHOD(HandsOffStorage)(void);

	// IOleControl methods
    //
    STDMETHOD(GetControlInfo)(LPCONTROLINFO pCI);
    STDMETHOD(OnMnemonic)(LPMSG pMsg);
    STDMETHOD(OnAmbientPropertyChange)(DISPID dispid);
    STDMETHOD(FreezeEvents)(BOOL bFreeze);

	// IOleObject implementation (yikes)
	STDMETHOD(SetClientSite)(IOleClientSite* pcs);
	STDMETHOD(GetClientSite)(IOleClientSite** ppcs);
	STDMETHOD(SetHostNames)(LPCOLESTR szContainerApp, LPCOLESTR szContainerObj);
	STDMETHOD(Close)(DWORD dwSaveOption);
	STDMETHOD(SetMoniker)(DWORD dwWitchMoniker, IMoniker* pmk);
	STDMETHOD(GetMoniker)(DWORD dwAssign, DWORD dwWitchMoniker, IMoniker** ppMoniker);
	STDMETHOD(InitFromData)(IDataObject* pDataObject, BOOL fCreation, DWORD dwReserved);
	STDMETHOD(IsUpToDate)();
	STDMETHOD(GetClipboardData)(DWORD dwReserved, IDataObject** ppDataObject);
	STDMETHOD(DoVerb)(LONG iVerb, LPMSG lpmsg, IOleClientSite* pActiveSite,
		LONG lIndex, HWND hwndParent, LPCRECT lprcPosRect);
	STDMETHOD(EnumVerbs)(IEnumOLEVERB** ppEnumOleVerb);
	STDMETHOD(Update)();
	STDMETHOD(GetUserClassID)(CLSID* pClsid);
	STDMETHOD(GetUserType)(DWORD dwFormOfType, LPOLESTR* pszUserType);
	STDMETHOD(SetExtent)(DWORD dwAspect, SIZEL* pSizel);
	STDMETHOD(GetExtent)(DWORD dwAspect, SIZEL* pSizel);
	STDMETHOD(Advise)(IAdviseSink* pAdvSink, DWORD* pdwConnection);
	STDMETHOD(Unadvise)(DWORD dwConnection);
	STDMETHOD(EnumAdvise)(IEnumSTATDATA** ppEnumAdvise);
	STDMETHOD(GetMiscStatus)(DWORD dwAspect, DWORD* pdwStatus);
	STDMETHOD(SetColorScheme)(LOGPALETTE* pLogpal);

	// IOleInPlaceObject implementation
	STDMETHOD(GetWindow)(HWND* phwnd);
	STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode);
	STDMETHOD(InPlaceDeactivate)();
	STDMETHOD(UIDeactivate)();
	STDMETHOD(SetObjectRects)(LPCRECT lprcPosRect, LPCRECT lprcClipRect);
	STDMETHOD(ReactivateAndUndo)();

	// IOleInPlaceActiveObject
    STDMETHOD(TranslateAccelerator)(LPMSG lpmsg);
    STDMETHOD(OnFrameWindowActivate)(BOOL fActivate);
    STDMETHOD(OnDocWindowActivate)(BOOL fActivate);
    STDMETHOD(ResizeBorder)(LPCRECT prcBorder,
            IOleInPlaceUIWindow *pUIWindow,
            BOOL fFrameWindow);
    STDMETHOD(EnableModeless)(BOOL fEnable);

	// IViewObject2
	STDMETHOD(Draw)(DWORD dwDrawAspect, LONG lindex, void  *pvAspect,
                    DVTARGETDEVICE  *ptd, HDC hdcTargetDev, HDC hdcDraw,
                    LPCRECTL lprcBounds, LPCRECTL lprcWBounds,
                    BOOL ( __stdcall  *pfnContinue )(ULONG_PTR dwContinue),
                    ULONG_PTR dwContinue);
    STDMETHOD(GetColorSet)(DWORD dwDrawAspect,LONG lindex, void  *pvAspect,
                           DVTARGETDEVICE  *ptd, HDC hicTargetDev,
                           LOGPALETTE  * *ppColorSet);
    STDMETHOD(Freeze)(DWORD dwDrawAspect, LONG lindex,
                      void  *pvAspect,DWORD  *pdwFreeze);
    STDMETHOD(Unfreeze)(DWORD dwFreeze);
    STDMETHOD(SetAdvise)(DWORD aspects, DWORD advf, IAdviseSink  *pAdvSink);
    STDMETHOD(GetAdvise)(DWORD *pAspects, DWORD  *pAdvf, IAdviseSink  * *ppAdvSink);
    STDMETHOD(GetExtent)(DWORD dwDrawAspect, LONG lindex, DVTARGETDEVICE __RPC_FAR *ptd, LPSIZEL lpsizel);

    // IQuickActivate methods
    //
    STDMETHOD(QuickActivate)(QACONTAINER *pqacontainer, QACONTROL *pqacontrol);
    STDMETHOD(SetContentExtent)(LPSIZEL);
    STDMETHOD(GetContentExtent)(LPSIZEL);

	HRESULT InPlaceActivate(BOOL lVerb);
    void SetInPlaceVisible(BOOL fShow);
    HWND CreateInPlaceWindow(int x, int y, BOOL fNoRedraw);
	void DestroyWindow();

	static tLuaControl *CreateLuaControl(
      lua_State* L,
      ITypeInfo* typeinfo,
      int ref
    );
private:
    IOleAdviseHolder *m_pOleAdviseHolder;          // IOleObject::Advise holder object
    IAdviseSink      *m_pViewAdviseSink;           // IViewAdvise sink for IViewObject2

	IOleClientSite     *m_pClientSite;             // client site
    IOleControlSite    *m_pControlSite;            // IOleControlSite ptr on client site
    IOleInPlaceSite    *m_pInPlaceSite;            // IOleInPlaceSite for managing activation
    IOleInPlaceFrame   *m_pInPlaceFrame;           // IOleInPlaceFrame ptr on client site
    IOleInPlaceUIWindow *m_pInPlaceUIWindow;       // for negotiating border space with client
    ISimpleFrameSite   *m_pSimpleFrameSite;        // simple frame site
    IDispatch          *m_pDispAmbient;            // ambient dispatch pointer
    SIZEL               m_Size;                    // the size of this control    
    RECT                m_rcLocation;              // where we at
    HWND                m_hwnd;                    // our window
    HWND                m_hwndParent;              // our parent window
    HRGN                m_hRgn;

    unsigned m_fDirty:1;                           // does the control need to be resaved?
    unsigned m_fInPlaceActive:1;                   // are we in place active or not?
    unsigned m_fInPlaceVisible:1;                  // we are in place visible or not?
    unsigned m_fUIActive:1;                        // are we UI active or not.
    unsigned m_fCreatingWindow:1;                  // indicates if we're in CreateWindowEx or not
    unsigned m_fUsingWindowRgn:1;                  // for SetObjectRects and clipping
    unsigned m_fViewAdvisePrimeFirst: 1;           // for IViewobject2::setadvise
    unsigned m_fViewAdviseOnlyOnce: 1;             // for iviewobject2::setadvise
};

class CEnumOLEVERB : public  IEnumOLEVERB {
    ULONG _refCount;
    int _nextVerb;
    const OLEVERB* _verbs;
    int _size;
public:
    CEnumOLEVERB(const OLEVERB* verbs, int size) : _refCount(0),
        _nextVerb(0), _size(size), _verbs(verbs) {
    }
    ~CEnumOLEVERB() { }
    ULONG _stdcall AddRef() { return ++_refCount; }
    ULONG _stdcall Release() {
        ULONG ret(_refCount); if(!ret) delete this; return ret;
    }
    HRESULT _stdcall QueryInterface(REFIID riid, void** ppv) {
        if(!ppv) return E_POINTER;
        if(riid == IID_IUnknown) *ppv = this;
        else if(riid == IID_IEnumOLEVERB) *ppv = this;
        else return *ppv = 0, E_NOINTERFACE;
        return AddRef(), S_OK;
    }

    static HRESULT CreateInstance(const OLEVERB* verbs, int size,
        IUnknown* pOuter, REFIID riid, void** ppv) {
        if(!ppv) return E_POINTER;
        if(pOuter) return *ppv = 0, CLASS_E_NOAGGREGATION;
        CEnumOLEVERB* pEnum = new CEnumOLEVERB(verbs, size);
        pEnum->AddRef();
        HRESULT hr = pEnum->QueryInterface(riid, ppv);
        pEnum->Release();
        return S_OK;
    }

    static HRESULT CreateInstance(const OLEVERB* verbs, int size,
        IEnumOLEVERB** ppEnumOLEVERB) {
        return CreateInstance(verbs, size, 0, IID_IEnumOLEVERB,
            reinterpret_cast<void**>(ppEnumOLEVERB));
    }

    HRESULT _stdcall Next(ULONG celt, OLEVERB* rgelt, ULONG* pceltFetched) {
        if(!rgelt) return E_POINTER;
        ULONG curPos(0);
        for(int nextVerb(_nextVerb); (nextVerb < _size) && (curPos < celt);
            ++nextVerb, ++curPos) {
            rgelt[curPos] = _verbs[nextVerb];
            rgelt[curPos].lpszVerbName = static_cast<wchar_t*>(
                CoTaskMemAlloc(lstrlenW(_verbs[nextVerb].lpszVerbName) * 2 + 2));
            lstrcpyW(rgelt[curPos].lpszVerbName, _verbs[nextVerb].lpszVerbName);
        }
        _nextVerb += curPos;
        if(pceltFetched) *pceltFetched = curPos;
        return (curPos == celt) ? S_OK : S_FALSE;
    }

    HRESULT _stdcall Clone(IEnumOLEVERB** ppenum) {
        HRESULT hr = CreateInstance(_verbs, _size, 0, IID_IEnumOLEVERB,
            reinterpret_cast<void**>(ppenum));
        if(hr) return hr;
        (*ppenum)->Skip(_nextVerb);
        return S_OK;
    }

    HRESULT _stdcall Reset() {
        return _nextVerb = 0, S_OK;
    }

    HRESULT _stdcall Skip(ULONG celt) {
        _nextVerb = min(_size, _nextVerb + static_cast<int>(celt));
        return S_OK;
    }
};

#endif // __TLUACONTROL_H
