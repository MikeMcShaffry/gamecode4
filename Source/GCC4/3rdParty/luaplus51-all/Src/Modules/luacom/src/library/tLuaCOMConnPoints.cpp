#include <windows.h>
#include <ole2.h>
#include <olectl.h>
#include <ocidl.h>

#include "tLuaCOM.h"
#include "tLuaCOMConnPoints.h"
#include "tCOMUtil.h"
#include "tLuaCOMException.h"
#include "LuaAux.h"

#include "luacom_internal.h"

extern "C"
{
#include "LuaCompat.h"
}

#define CONNPOINT_NAME "__LUACOM_CONNPOINT"


/*---------------------------------------------------------------------------
  tLuaCOMEnumConnPoints's implementation of its main COM object class
  including Constructor, Destructor, QueryInterface, AddRef, Release,
  Next, Skip, Reset, and Clone.
---------------------------------------------------------------------------*/

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMEnumConnPoints::tLuaCOMEnumConnPoints

  Summary:  tLuaCOMEnumConnPoints Constructor.

  Args:     IUnknown* pHostObj
              Pointer to the host object whose connection points are
              being enumerated.

  Modifies: cRefs, pHostObj, iEnumIndex, cConnPts, and paConnPts.

  Returns:  void
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
tLuaCOMEnumConnPoints::tLuaCOMEnumConnPoints(IUnknown* pHostObj)
{
  // Zero the COM object's reference count.
  cRefs = 0;

  // Assign the Host Object pointer.
  pHostObj = pHostObj;

  // Initialize the Connection Point enumerator variables.
  iEnumIndex = 0;
  cConnPts = 0;
  paConnPts = NULL;

  return;
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMEnumConnPoints::~tLuaCOMEnumConnPoints

  Summary:  tLuaCOMEnumConnPoints Destructor.

  Args:     void

  Modifies: paConnPts.

  Returns:  void
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
tLuaCOMEnumConnPoints::~tLuaCOMEnumConnPoints(void)
{
  if (NULL != paConnPts)
  {
    UINT i;

    // Release all the connection point interface pointers.
    for (i=0; i<cConnPts; i++)
      if (NULL != paConnPts[i])
        COM_RELEASE(paConnPts[i]);

    // Delete the array of interface pointers.
    delete [] paConnPts;
  }

  return;
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMEnumConnPoints::Init

  Summary:  tLuaCOMEnumConnPoints Initialization method.  Create any
            necessary arrays, structures, and objects.

  Args:     ULONG cConnPts,
              Number of Connections Points.
            IConnPoint** paConnPts,
              Pointer to array of connection point interface pointers.
            ULONG iEnumIndex
              The Enumerator index initial value.

  Modifies: cConnPts, paConnPts, iEnumIndex.

  Returns:  HRESULT
              Standard result code. NOERROR for success.
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
HRESULT tLuaCOMEnumConnPoints::Init(
          ULONG cConnPts,
          IConnectionPoint** paConnPts,
          ULONG iEnumIndex)
{
  HRESULT hr = NOERROR;
  UINT i;

  // Remember the number of Connection points.
  cConnPts = cConnPts;

  // Remember the initial enumerator index.
  iEnumIndex = iEnumIndex;

  // Create a copy of the array of connection points and keep it inside
  // this enumerator COM object.
  paConnPts = new IConnectionPoint* [(UINT) cConnPts];

  // Fill the array copy with the IConnPoint interface pointers from
  // the array passed. AddRef for each new Interface pointer copy made.
  if (NULL != paConnPts)
  {
    for (i=0; i<cConnPts; i++)
    {
      paConnPts[i] = paConnPts[i];
      paConnPts[i]->AddRef();
    }
  }
  else
    hr = E_OUTOFMEMORY;

  return (hr);
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMEnumConnPoints::QueryInterface

  Summary:  QueryInterface of the tLuaCOMEnumConnPoints non-delegating
            IUnknown implementation.

  Args:     REFIID riid,
              [in] GUID of the Interface being requested.
            void ** ppv)
              [out] Address of the caller's pointer variable that will
              receive the requested interface pointer.

  Modifies: .

  Returns:  HRESULT
              Standard result code. NOERROR for success.
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
STDMETHODIMP tLuaCOMEnumConnPoints::QueryInterface(
               REFIID riid,
               void ** ppv)
{
  HRESULT hr = E_NOINTERFACE;

  *ppv = NULL;

  // The IEnumConnectionPoints interface is implemented directly in
  // this COM object rather than being a nested interface implementation.
  if (IID_IUnknown == riid || IID_IEnumConnectionPoints == riid)
    *ppv = (LPVOID)this;

  if (NULL != *ppv)
  {
    // We've handed out a pointer to the interface so obey the COM rules
    // and AddRef the reference count.
    ((LPUNKNOWN)*ppv)->AddRef();
    hr = NOERROR;
  }

  return (hr);
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMEnumConnPoints::AddRef

  Summary:  AddRef of the tLuaCOMEnumConnPoints non-delegating IUnknown
            implementation.

  Args:     void

  Modifies: cRefs.

  Returns:  ULONG
              New value of cRefs (COM object's reference count).
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
STDMETHODIMP_(ULONG) tLuaCOMEnumConnPoints::AddRef(void)
{
  cRefs = ++cRefs;

  // Also AddRef the host object to ensure it stays around as long as
  // this enumerator.
  pHostObj->AddRef();

  return cRefs;
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMEnumConnPoints::Release

  Summary:  Release of the tLuaCOMEnumConnPoints non-delegating IUnknown
            implementation.

  Args:     void

  Modifies: cRefs.

  Returns:  ULONG
              New value of cRefs (COM object's reference count).
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
STDMETHODIMP_(ULONG) tLuaCOMEnumConnPoints::Release(void)
{
  // Pass this release along to the Host object being enumerated.
  pHostObj->Release();

  cRefs = --cRefs;

  if (0 == cRefs)
  {
    // We artificially bump the main ref count to prevent reentrancy via
    // the main object destructor.
    cRefs++;
    delete this;
  }

  return cRefs;
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMEnumConnPoints::Next

  Summary:  The Next member method of this IEnumConnectionPoints interface
            implementation. Called by outside clients of a
            tLuaCOMEnumConnPoints object to request that a number of next
            connection point interface pointers be deposited into an array
            supplied by the caller.

  Args:     ULONG cReq
              Number of connection points requested for return (starting at
              the current Enumerator index).
            IConnPoint** paConnPts,
              Pointer to a caller's output array that will receive the
              enumerated IConnPoint interface pointers.
            ULONG* cEnumerated)
              Pointer to a ULONG variable that will contain the number of
              connection points actually enumerated by this call.

  Modifies: .

  Returns:  HRESULT
              Standard result code. NOERROR for success.
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
STDMETHODIMP tLuaCOMEnumConnPoints::Next(
               ULONG cReq,
               IConnectionPoint** paConnPts,
               ULONG* pcEnumerated)
{
  HRESULT hr = NOERROR;
  ULONG cRet = 0;

  // Make sure the argument values passed are valid.
  if (NULL != paConnPts)
  {
    if (NULL != paConnPts)
    {
      if (NULL != *paConnPts && iEnumIndex < cConnPts)
      {
        if (NULL != pcEnumerated)
          *pcEnumerated = 0L;
        else
          if (1L != cReq)
            hr = E_POINTER;
      }
      else
        hr = S_FALSE;
    }
    else
      hr = E_POINTER;
  }
  else
    hr = S_FALSE;

  if (SUCCEEDED(hr))
  {
    // Starting at the current Enumerator index, loop to assign the
    // requested number of output connection point interface pointers.
    for (; iEnumIndex < cConnPts && cReq > 0;
           paConnPts++, cRet++, cReq--)
    {
      // Assign from the inside Enumerator array to the specified receiving
      // array.
      *paConnPts = paConnPts[iEnumIndex++];
      // After assigning a copy of an IConnPoint pointer, AddRef it.
      if (NULL != *paConnPts)
        (*paConnPts)->AddRef();
    }

    // Assign the output number of connection points enumerated.
    if (NULL != pcEnumerated)
      *pcEnumerated = cRet;
  }

  return hr;
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMEnumConnPoints::Skip

  Summary:  The Skip member method of this IEnumConnectionPoints interface
            implementation. Starting at the current Enumerator index, skip
            the specified number of connection point items.

  Args:     ULONG cSkip
              Number of Connection Point items to skip.

  Modifies: .

  Returns:  HRESULT
              Standard result code. NOERROR for success.
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
STDMETHODIMP tLuaCOMEnumConnPoints::Skip(
               ULONG cSkip)
{
  HRESULT hr = NOERROR;

  // If there really is a connection point array and the requested
  // amount of skip does not exceed the number of connection points,
  // then bump the index by the requested skip amount.
  if (NULL != paConnPts && (iEnumIndex + cSkip) < cConnPts)
    iEnumIndex += cSkip;
  else
    hr = S_FALSE;

  return hr;
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMEnumConnPoints::Reset

  Summary:  The Reset member method of the IEnumConnectionPoints interface
            implementation. Resets the Enumeration index to the first
            connection point item in the array.

  Args:     void.

  Modifies: .

  Returns:  HRESULT
              Standard result code. NOERROR for success.
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
STDMETHODIMP tLuaCOMEnumConnPoints::Reset(
               void)
{
  // Zero the main Enumerator index.
  iEnumIndex = 0;

  return NOERROR;
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMEnumConnPoints::Clone

  Summary:  The Clone member method of this IEnumConnectionPoints
            interface implementation. Creates a new clone of this entire
            Connection Point enumerator COM object.

  Args:     IEnumConnectionPoints** ppIEnum
              Address of the caller's output pointer variable that will
              receive the IEnumConnectionPoints interface pointer of the
              new enumerator clone.

  Modifies: ...

  Returns:  HRESULT
              Standard result code. NOERROR for success.
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
STDMETHODIMP tLuaCOMEnumConnPoints::Clone(
               IEnumConnectionPoints** ppIEnum)
{
  HRESULT hr;
  tLuaCOMEnumConnPoints* pCOEnum;

  // NULL the output variable first.
  *ppIEnum = NULL;

  // Create the Clone Enumerator COM object.
  pCOEnum = new tLuaCOMEnumConnPoints(pHostObj);
  if (NULL != pCOEnum)
  {
    // Initialize it with same values as in this existing enumerator.
    hr = pCOEnum->Init(cConnPts, paConnPts, iEnumIndex);
    if (SUCCEEDED(hr))
    {
      // QueryInterface to return the requested interface pointer.
      // An AddRef will be conveniently done by the QI.
      if (SUCCEEDED(hr))
        hr = pCOEnum->QueryInterface(
                        IID_IEnumConnectionPoints,
                        (void **)ppIEnum);
    }
  }
  else
    hr = E_OUTOFMEMORY;

  return hr;
}


/*---------------------------------------------------------------------------
  tLuaCOMConnPoint's implementation of its main COM object class
  including Constructor, Destructor, QueryInterface, AddRef, Release,
  GetConnectionInterface, GetConnPointContainer, Advise, Unadvise,
  and EnumConnections.
---------------------------------------------------------------------------*/

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMConnPoint::tLuaCOMConnPoint

  Summary:  tLuaCOMConnPoint Constructor.

  Args:     IUnknown* pHostObj
              Pointer to IUnknown of the connectable object offering this
              connection point.

  Modifies: ...

  Returns:  void
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
tLuaCOMConnPoint::tLuaCOMConnPoint(lua_State *p_L,
                                   IUnknown* pHostObj)
{
  // stores lua state
  L = p_L;

  // Zero the COM object's reference count.
  cRefs = 0;

  // Remember an IUnknown pointer to the connectable object that offers
  // this connection point. Since this connection point object's lifetime
  // is geared to that of the connectable object there is no need to
  // AddRef the following copied pointer to the connectable object.
  pHostObj = pHostObj;

  // Initialize the Connection Point variables.
  dwNextCookie = COOKIE_START_VALUE;
  uiMaxIndex = 0;
  cConnections = 0;

  // creates a new lua tag associated with this connection point
  luaCompat_pushTypeByName(L, MODULENAME, LCOM_CONNPOINT_TYPENAME);
  lua_pushcfunction(L, tagmeth_index);
  luaCompat_handleNoIndexEvent(L);

  lua_pop(L, 1);

  for(int i = 0; i < ALLOC_CONNECTIONS; i++)
    sinks[i] = NULL;


  return;
}


tLuaCOMConnPoint::~tLuaCOMConnPoint(void)
{
  UINT i;

  if (NULL != paConnections)
  {
    // Release all the connection sink interface pointers.
    for (i=0; i<uiMaxIndex; i++)
    {
      if (NULL != paConnections[i].pUnk)
        paConnections[i].pUnk->Release();

      if (NULL != sinks[i])
      {
        sinks[i]->Unlock();
        sinks[i] = NULL;
      }
    }
  }

  return;
}


HRESULT tLuaCOMConnPoint::Init(REFIID riid, ITypeInfo *typeinfo)
{
  HRESULT hr = NOERROR;
  CONNECTDATA* paConns = paConnections;

  // Keep a copy of the reference to the IID of the sink interface
  // associated with this connection point. Needed for later
  // use by the GetConnectionInterface method.
  iidSink = riid;
  source_typeinfo = typeinfo;

  // Build the initial dynamic array for connections.
/*  paConns = new CONNECTDATA[ALLOC_CONNECTIONS];
  sinks = new tLuaCOM*[ALLOC_CONNECTIONS];*/

  if (NULL != paConns)
  {
    // Zero the array.
    memset(paConns, 0, ALLOC_CONNECTIONS * sizeof(CONNECTDATA));

    // Rig this connection point object so that it will use the
    // new internal array of connections.
    uiMaxIndex = ALLOC_CONNECTIONS;
    //paConnections = paConns;
  }
  else
    hr = E_OUTOFMEMORY;

  return (hr);
}



STDMETHODIMP tLuaCOMConnPoint::QueryInterface(
               REFIID riid,
               void ** ppv)
{
  HRESULT hr = E_NOINTERFACE;

  *ppv = NULL;

  // The IConnPoint interface is implemented directly in this
  // COM object rather than being a nested interface implementation.
  if (IID_IUnknown == riid || IID_IConnectionPoint == riid)
    *ppv = (LPVOID)this;

  if (NULL != *ppv)
  {
    // We've handed out a pointer to the interface so obey the COM rules
    // and AddRef the reference count.
    ((LPUNKNOWN)*ppv)->AddRef();
    hr = NOERROR;
  }

  return (hr);
}


STDMETHODIMP_(ULONG) tLuaCOMConnPoint::AddRef(void)
{
  cRefs = ++cRefs;

  return cRefs;
}


STDMETHODIMP_(ULONG) tLuaCOMConnPoint::Release(void)
{
  cRefs = --cRefs;

    if (0 == cRefs)
    {
      // We artificially bump the main ref count to prevent reentrancy via
      // the main object destructor. We relinquish thread ownership of this
      // object prior to deleting it--a good practice.
      cRefs++;
      delete this;
    }

  return cRefs;
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMConnPoint::GetSlot

  Summary:  An internal private utility member method to obtain a free
            slot in the dynamic connections array. GetSlot will expand the
            dynamic array for more entries if needed. To guarantee thread
            safety, this private method should always be called within the
            protection of a bracketed OwnThis, UnOwnThis pair.

  Args:     UINT* puiFreeSlot
              Address of an output variable to receive the free slot index.

  Modifies: uiMaxIndex, paConnections.

  Returns:  HRESULT
              Standard result code. NOERROR for success.
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
HRESULT tLuaCOMConnPoint::GetSlot(
          UINT* puiFreeSlot)
{
  HRESULT hr = NOERROR;
  BOOL bOpen = FALSE;
  UINT i;
  //CONNECTDATA* paConns;

  // Zero the output variable.
  *puiFreeSlot = 0;

  // Loop to find an empty slot.
  for (i=0; i<uiMaxIndex; i++)
  {
    if (paConnections[i].dwCookie == 0)
    {
      // We found an open empty slot.
      *puiFreeSlot = i;
      bOpen = TRUE;
      break;
    }
  }

  if (!bOpen)
  {
    hr = E_OUTOFMEMORY;
  }

  return hr;
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMConnPoint::FindSlot

  Summary:  An internal private utility member method to find an existing
            slot (identified by the specified dwCookie value) in the
            dynamic connections array.

  Args:     DWORD dwCookie,
              The connection key (cookie) to find.
            UINT* puiSlot)
              Address of an output variable to receive the slot index.

  Modifies: ...

  Returns:  HRESULT
              Standard result code. NOERROR for success.
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
HRESULT tLuaCOMConnPoint::FindSlot(
          DWORD dwCookie,
          UINT* puiSlot)
{
  HRESULT hr = CONNECT_E_NOCONNECTION;
  UINT i;

  // Loop to find the Cookie.
  for (i=0; i<uiMaxIndex; i++)
  {
    if (dwCookie == paConnections[i].dwCookie)
    {
      // If a cookie match is found, assign the output slot index.
      *puiSlot = i;
      hr = NOERROR;
      break;
    }
  }

  return hr;
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMConnPoint::GetConnectionInterface

  Summary:  The GetConnectionInterface member method of this
            IConnPoint interface implementation. Called to get the
            IID of the Sink interface associated with this connection
            point.

  Args:     IID* piidSink
              Pointer to the IID of the associated sink interface.

  Modifies: .

  Returns:  HRESULT
              Standard result code. NOERROR for success.
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
STDMETHODIMP tLuaCOMConnPoint::GetConnectionInterface(
               IID* piidSink)
{
  HRESULT hr = NOERROR;

  if (NULL != piidSink)
    *piidSink = iidSink;
  else
    hr = E_POINTER;

  return hr;
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMConnPoint::GetConnPointContainer

  Summary:  The GetConnPointContainer member method of this
            IConnPoint interface implementation. Called to get the
            connection point container that contains this connection
            point.

  Args:     IConnPointContainer** ppConnPtCon
              Address of the pointer variable that will recieve the
              IConnPointContainer interface pointer.

  Modifies: .

  Returns:  HRESULT
              Standard result code. NOERROR for success.
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
STDMETHODIMP tLuaCOMConnPoint::GetConnectionPointContainer(
               IConnectionPointContainer** ppConnPtCon)
{
  HRESULT hr;

  // Use QueryInterface to get the interface pointer and to perform the
  // needed AddRef on the returned pointer.
  hr = pHostObj->QueryInterface(
                     IID_IConnectionPointContainer,
                     (void **) ppConnPtCon);

  return hr;
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMConnPoint::Advise

  Summary:  The Advise member method of this IConnPoint interface
            implementation. Called by clients to establish a connection of
            their sink to this connection point. Uses the CThreaded
            OwnThis mechanism to provide mutually exclusive access by
            multiple threads.

  Args:     IUnknown* pUnkSink
              IUnknown interface pointer of the Sink object in the client.
            DWORD* pdwCookie
              Pointer to a DWORD in the client that will receive a unique
              key used by the client to refer to the connection established
              by this Advise call.

  Modifies: ...

  Returns:  HRESULT
              Standard result code. NOERROR for success.
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
STDMETHODIMP tLuaCOMConnPoint::Advise(
               IUnknown* pUnkSink,
               DWORD* pdwCookie)
{
  HRESULT hr = NOERROR;
  UINT uiFreeSlot = 0;
  IDispatch* pISink = NULL;


  // Zero the output connection key.
  *pdwCookie = 0;

  // Get the specific associated client Sink interface that this
  // connection point expects to use for notifications.
  hr = pUnkSink->QueryInterface(IID_IDispatch/*iidSink*/, (void **)&pISink);
  if (SUCCEEDED(hr))
  {
    // Store the specific sink interface in this connection point's
    // array of live connections. First find a free slot (expand the
    // array if needed).
    hr = GetSlot(&uiFreeSlot);
    if (SUCCEEDED(hr))
    {
      // Assign the new slot with the connection entry.
      paConnections[uiFreeSlot].pUnk = pISink;
      paConnections[uiFreeSlot].dwCookie = dwNextCookie;

      try
      {
        // vb supplies a very weird type info, so we
        // stay with ours, as it's where vb will look for the
        // DISPID's anyway
        sinks[uiFreeSlot] = 
          tLuaCOM::CreateLuaCOM(L, pISink, IID_NULL, source_typeinfo);
      }
      catch(class tLuaCOMException& e)
      {
        UNUSED(e);
        return CONNECT_E_CANNOTCONNECT;
      }

      // Assign the output Cookie value.
      *pdwCookie = dwNextCookie;

      // Increment the Cookie counter.
      dwNextCookie++;

      // Increment the number of live connections.
      cConnections++;
    }
  }
  else if (hr == E_NOINTERFACE)
  {
     // The sink does not support iidSink.
     hr = CONNECT_E_CANNOTCONNECT;
  }

  return hr;
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMConnPoint::Unadvise

  Summary:  The Unadvise member method of this IConnPoint interface
            implementation. Called by clients to disconnect a connection
            of their sink to this connection point. The connection is
            identified by the dwCookie argument (obtained by a previous
            Advise call). Uses the CThreaded OwnThis mechanism to provide
            mutually exclusive access by multiple threads.

  Args:     DWORD dwCookie
              Connection key that was obtained by a previous Advise call.

  Modifies: .

  Returns:  HRESULT
              Standard result code. NOERROR for success.
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
STDMETHODIMP tLuaCOMConnPoint::Unadvise(
               DWORD dwCookie)
{
  HRESULT hr = NOERROR;
  UINT uiSlot;

  if (0 != dwCookie)
  {
      hr = FindSlot(dwCookie, &uiSlot);
      if (SUCCEEDED(hr))
      {
        // Release the sink interface.
        COM_RELEASE(paConnections[uiSlot].pUnk);
        sinks[uiSlot]->Unlock();
        sinks[uiSlot] = NULL;

        // Mark the array entry as empty.
        paConnections[uiSlot].dwCookie = 0;

        // Decrement the number of live connections.
        cConnections--;
      }

  }
  else
    hr = E_INVALIDARG;

  return hr;
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMConnPoint::EnumConnections

  Summary:  The EnumConnections member method of this IConnPoint
            interface implementation. Called to obtain an IEnumConnections
            enumerator interface that can be used to enumerate the
            connections of this connection point. Uses the CThreaded
            OwnThis mechanism to ensure mutually exclusive access by
            multiple threads.

  Args:     IEnumConnections** ppIEnum
              Address of the caller's output pointer variable that will
              receive the enumerator IEnumConnections interface pointer.

  Modifies: ...

  Returns:  HRESULT
              Standard result code. NOERROR for success.
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
STDMETHODIMP tLuaCOMConnPoint::EnumConnections(
               IEnumConnections** ppIEnum)
{
  HRESULT hr = OLE_E_NOCONNECTION;
  CONNECTDATA* paConns;
  tLuaCOMEnumConnections* pCOEnum;
  UINT i,j;

    // Zero the output enumerator interface pointer.
    *ppIEnum = NULL;

    if (0 != cConnections)
    {
      // Create an array of CONNECTDATA structures.
      paConns = new CONNECTDATA[(UINT)cConnections];
      if (NULL != paConns)
      {
        for (i=0, j=0; i<uiMaxIndex && j<cConnections; i++)
        {
          // Copy non-empty entries only.
          if (0 != paConnections[i].dwCookie)
          {
            // Assign the occupied entry.
            paConns[j].pUnk = (IUnknown*)paConnections[i].pUnk;
            paConns[j].dwCookie = paConnections[i].dwCookie;
            j++;
          }
        }

        // Create a new COM object for enumerating connections. Pass
        // 'this' as a pHostObj pointer used later to ensure the host
        // connection point object stays alive as long as the enumerator
        // that enumerates connections to that connection point.
        pCOEnum = new tLuaCOMEnumConnections(this);
        if (NULL != pCOEnum)
        {
          // Use the previously constructed (paConns) array of connections
          // to init the new tLuaCOMEnumConnections COM object. The Init will
          // build yet another internal copy of this array. Set the
          // initial enumerator index to 0.
          hr = pCOEnum->Init(cConnections, paConns, 0);

          // QueryInterface to return the requested interface pointer.
          // An AddRef will be conveniently done by the QI.
          if (SUCCEEDED(hr))
            hr = pCOEnum->QueryInterface(
                            IID_IEnumConnections,
                            (void **)ppIEnum);
        }
        else
          hr = E_OUTOFMEMORY;

        // We're done with the locally constructed array copy--delete it.
        delete [] paConns;
      }
      else
        hr = E_OUTOFMEMORY;
    }

  return hr;
}


/*---------------------------------------------------------------------------
  tLuaCOMEnumConnections's implementation of its main COM object class
  including Constructor, Destructor, QueryInterface, AddRef, Release,
  Next, Skip, Reset, and Clone.
---------------------------------------------------------------------------*/

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMEnumConnections::tLuaCOMEnumConnections

  Summary:  tLuaCOMEnumConnections Constructor.

  Args:     IUnknown* pHostObj
              Pointer to IUnknown interface of the host Connection Point
              COM object whose connections are being enumerated.

  Modifies: cRefs, pHostObj, iEnumIndex, cConnections,
            and paConnections.

  Returns:  void
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
tLuaCOMEnumConnections::tLuaCOMEnumConnections(
  IUnknown* pHostObj)
{
  // Zero the COM object's reference count.
  cRefs = 0;

  // Assign the Host Object pointer.
  pHostObj = pHostObj;

  // Initialize the Connection Point enumerator variables.
  iEnumIndex = 0;
  cConnections = 0;
  paConnections = NULL;

  return;
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMEnumConnections::~tLuaCOMEnumConnections

  Summary:  tLuaCOMEnumConnections Destructor.

  Args:     void

  Modifies: paConnections.

  Returns:  void
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
tLuaCOMEnumConnections::~tLuaCOMEnumConnections(void)
{
  if (NULL != paConnections)
  {
    UINT i;

    // Release all the connected sink interface pointers.
    for (i=0; i<cConnections; i++)
      paConnections[i].pUnk->Release();

    // Delete the array of connections.
    delete [] paConnections;
  }

  return;
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMEnumConnections::Init

  Summary:  tLuaCOMEnumConnections Initialization method.  Create any
            necessary arrays, structures, and objects.

  Args:     ULONG cConnections
              Number of Connections.
            CONNECTDATA* paConnections,
              Pointer to array of connections.
            ULONG iEnumIndex
              The Enumerator index initial value.

  Modifies: cConnections, paConnections, pHostObj, iEnumIndex.

  Returns:  HRESULT
              Standard result code. NOERROR for success.
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
HRESULT tLuaCOMEnumConnections::Init(
          ULONG cConnections,
          CONNECTDATA* paConnections,
          ULONG iEnumIndex)
{
  HRESULT hr = NOERROR;
  UINT i;

  // Remember the number of live Connections.
  cConnections = cConnections;

  // Remember the initial enumerator index.
  iEnumIndex = iEnumIndex;

  // Create a copy of the array of connections and keep it inside
  // this enumerator COM object.
  paConnections = new CONNECTDATA [(UINT) cConnections];

  // Fill the array copy with the connection data from the connections
  // array passed. AddRef for each new sink Interface pointer copy made.
  if (NULL != paConnections)
  {
    for (i=0; i<cConnections; i++)
    {
      paConnections[i] = paConnections[i];
      paConnections[i].pUnk->AddRef();
    }
  }
  else
    hr = E_OUTOFMEMORY;

  return (hr);
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMEnumConnections::QueryInterface

  Summary:  QueryInterface of the tLuaCOMEnumConnections non-delegating
            IUnknown implementation.

  Args:     REFIID riid,
              [in] GUID of the Interface being requested.
            void ** ppv)
              [out] Address of the caller's pointer variable that will
              receive the requested interface pointer.

  Modifies: .

  Returns:  HRESULT
              Standard result code. NOERROR for success.
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
STDMETHODIMP tLuaCOMEnumConnections::QueryInterface(
               REFIID riid,
               void ** ppv)
{
  HRESULT hr = E_NOINTERFACE;

  *ppv = NULL;

  // The IEnumConnections interface is implemented directly in
  // this COM object rather than being a nested interface implementation.
  if (IID_IUnknown == riid || IID_IEnumConnections == riid)
    *ppv = (LPVOID)this;

  if (NULL != *ppv)
  {
    // We've handed out a pointer to the interface so obey the COM rules
    // and AddRef the reference count.
    ((LPUNKNOWN)*ppv)->AddRef();
    hr = NOERROR;
  }

  return (hr);
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMEnumConnections::AddRef

  Summary:  AddRef of the tLuaCOMEnumConnections non-delegating IUnknown
            implementation.

  Args:     void

  Modifies: cRefs.

  Returns:  ULONG
              New value of cRefs (COM object's reference count).
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
STDMETHODIMP_(ULONG) tLuaCOMEnumConnections::AddRef(void)
{
  cRefs = ++cRefs;

  // Also AddRef the host object to ensure it stays around as long as
  // this enumerator.
  pHostObj->AddRef();

  return cRefs;
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMEnumConnections::Release

  Summary:  Release of the tLuaCOMEnumConnections non-delegating IUnknown
            implementation.

  Args:     void

  Modifies: cRefs.

  Returns:  ULONG
              New value of cRefs (COM object's reference count).
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
STDMETHODIMP_(ULONG) tLuaCOMEnumConnections::Release(void)
{
  // Pass this release along to the Host object being enumerated.
  pHostObj->Release();

  cRefs = --cRefs;

  if (0 == cRefs)
  {
    // We artificially bump the main ref count to prevent reentrancy via
    // the main object destructor.
    cRefs++;
    delete this;
  }

  return cRefs;
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMEnumConnections::Next

  Summary:  The Next member method of this IEnumConnections interface
            implementation. Called by outside clients of a
            tLuaCOMEnumConnections object to request a number of next
            connections be returned in an array supplied by the caller.

  Args:     ULONG cReq
              Number of connection points requested for return (starting at
              the current Enumerator index).
            CONNECTDATA* paConnections,
              Pointer to a caller's output array that will receive the
              enumerated connection data structures.
            ULONG* pcEnumerated)
              Pointer to a ULONG variable that will contain the number of
              connection points actually enumerated by this call.

  Modifies: .

  Returns:  HRESULT
              Standard result code. NOERROR for success.
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
STDMETHODIMP tLuaCOMEnumConnections::Next(
               ULONG cReq,
               CONNECTDATA* paConnections,
               ULONG* pcEnumerated)
{
  HRESULT hr = NOERROR;
  ULONG cRet = 0;

  // Make sure the argument values passed are valid.
  if (NULL != paConnections)
  {
    if (NULL != paConnections)
    {
      if (iEnumIndex < cConnections)
      {
        if (NULL != pcEnumerated)
          *pcEnumerated = 0L;
        else
          if (1L != cReq)
            hr = E_POINTER;
      }
      else
        hr = S_FALSE;
    }
    else
      hr = E_POINTER;
  }
  else
    hr = S_FALSE;

  if (SUCCEEDED(hr))
  {
    // Starting at the current Enumerator index, loop to assign the
    // requested number of output connection data structures.
    for (; iEnumIndex < cConnections && cReq > 0;
           paConnections++, iEnumIndex++, cRet++, cReq--)
    {
      // Because we are assigning a copy of a connection's data, AddRef
      // its sink interface pointer.
      if (NULL != paConnections[iEnumIndex].pUnk)
        paConnections[iEnumIndex].pUnk->AddRef();

      // Assign a connection's data from the inside Enumerator array to
      // the specified output receiving array.
      *paConnections = paConnections[iEnumIndex];
    }

    // Assign the output number of connections enumerated.
    if (NULL != pcEnumerated)
      *pcEnumerated = cRet;
  }

  return hr;
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMEnumConnections::Skip

  Summary:  The Skip member method of this IEnumConnections interface
            implementation. Starting at the current Enumerator index, skip
            the specified number of connection items.

  Args:     ULONG cSkip
              Number of Connection items to skip.

  Modifies: .

  Returns:  HRESULT
              Standard result code. NOERROR for success.
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
STDMETHODIMP tLuaCOMEnumConnections::Skip(
               ULONG cSkip)
{
  HRESULT hr = NOERROR;

  // If there really is a connection array and the requested
  // amount of skip does not exceed the number of connections,
  // then bump the index by the requested skip amount.
  if (NULL != paConnections && (iEnumIndex + cSkip) < cConnections)
    iEnumIndex += cSkip;
  else
    hr = S_FALSE;

  return hr;
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMEnumConnections::Reset

  Summary:  The Reset member method of the IEnumConnections interface
            implementation. Resets the Enumeration index to the first
            connection item in the array.

  Args:     void.

  Modifies: .

  Returns:  HRESULT
              Standard result code. NOERROR for success.
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
STDMETHODIMP tLuaCOMEnumConnections::Reset(
               void)
{
  // Zero the main Enumerator index.
  iEnumIndex = 0;

  return NOERROR;
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMEnumConnections::Clone

  Summary:  The Clone member method of this IEnumConnections interface
            implementation. Creates a new clone of this entire Connection
            enumerator COM object and returns a pointer to its
            IEnumConnections interface.

  Args:     IEnumConnections** ppIEnum
              Address of the caller's output pointer variable that will
              receive the IEnumConnections interface pointer of the
              new enumerator clone.

  Modifies: ...

  Returns:  HRESULT
              Standard result code. NOERROR for success.
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
STDMETHODIMP tLuaCOMEnumConnections::Clone(
               IEnumConnections** ppIEnum)
{
  HRESULT hr;
  tLuaCOMEnumConnections* pCOEnum;

  // NULL the output variable first.
  *ppIEnum = NULL;

  // Create the Clone Enumerator COM object.
  pCOEnum = new tLuaCOMEnumConnections(pHostObj);
  if (NULL != pCOEnum)
  {
    // Initialize it with same values as in this existing enumerator.
    hr = pCOEnum->Init(cConnections, paConnections, iEnumIndex);
    if (SUCCEEDED(hr))
    {
      // QueryInterface to return the requested interface pointer.
      // An AddRef will be conveniently done by the QI.
      if (SUCCEEDED(hr))
        hr = pCOEnum->QueryInterface(
                        IID_IEnumConnections,
                        (void **)ppIEnum);
    }
  }
  else
    hr = E_OUTOFMEMORY;

  return hr;
}














/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMConnPointContainer
              ::tLuaCOMConnPointContainer

  Summary:  Constructor for the tLuaCOMConnPointContainer interface
            instantiation.

  Args:     COBall* pBackObj,
              Back pointer to the parent outer object.
            IUnknown* pUnkOuter
              Pointer to the outer Unknown.  For delegation.

  Modifies: pBackObj, pUnkOuter.

  Returns:  void
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
tLuaCOMConnPointContainer::tLuaCOMConnPointContainer(lua_State* p_L,
                                                     IUnknown* p_pUnkOuter)
{
  HRESULT hr = S_OK;
  ITypeInfo *events_typeinfo = NULL;

  CHECKPARAM(p_L); CHECKPARAM(p_pUnkOuter);

  // stores pointers
  pUnkOuter = p_pUnkOuter;
  L = p_L;

  // creates connection point for source interface
  IProvideClassInfo2 *ci2  = NULL;

  hr = pUnkOuter->QueryInterface(IID_IProvideClassInfo2, (void **) &ci2); 
  CHK_COM_CODE(hr);

  IID iid;

  hr = ci2->GetGUID(GUIDKIND_DEFAULT_SOURCE_DISP_IID, &iid);
  CHK_COM_CODE(hr);

  ITypeInfo *coclassinfo = NULL;

  hr = ci2->GetClassInfo(&coclassinfo);
  CHK_COM_CODE(hr);  

  events_typeinfo = tCOMUtil::GetDefaultInterfaceTypeInfo(coclassinfo, true);
  CHK_LCOM_ERR(events_typeinfo, "No default source typeinfo.");

  COM_RELEASE(ci2);
  COM_RELEASE(coclassinfo);

  max_connection_points = 1;
  connection_points = new tLuaCOMConnPoint*[max_connection_points];
  CHKMALLOC(connection_points);

  connection_points[0] = new tLuaCOMConnPoint(L, p_pUnkOuter);
  CHKMALLOC(connection_points[0]);

  connection_points[0]->AddRef();
  
  num_connection_points = 1;

  hr = connection_points[0]->Init(iid, events_typeinfo);
  CHK_COM_CODE(hr);  

  default_connection = connection_points[0];

  return;
}



tLuaCOMConnPointContainer::~tLuaCOMConnPointContainer(void)
{
  while(num_connection_points--)
  {
    COM_RELEASE(connection_points[num_connection_points]);
  }

  delete[] connection_points;
  return;
}


STDMETHODIMP tLuaCOMConnPointContainer::QueryInterface(
               REFIID riid,
               void ** ppv)
{
  // Delegate this call to the outer object's QueryInterface.
  return pUnkOuter->QueryInterface(riid, ppv);
}



STDMETHODIMP_(ULONG) tLuaCOMConnPointContainer::AddRef(void)
{
  return pUnkOuter->AddRef();
}


STDMETHODIMP_(ULONG) tLuaCOMConnPointContainer::Release(void)
{
  return pUnkOuter->Release();
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMConnPointContainer::FindConnectionPoint

  Summary:  Given an IID for a connection point sink find and return the
            interface pointer for that connection point sink.

  Args:     REFIID riid
              Reference to an IID
            IConnectionPoint** ppConnPt
              Pointer to the caller's Connection Point pointer variable.

  Modifies: .

  Returns:  HRESULT
              Standard result code. NOERROR for success.
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
STDMETHODIMP tLuaCOMConnPointContainer::FindConnectionPoint(
               REFIID riid,
               IConnectionPoint** ppConnPt)
{
  HRESULT hr = E_NOINTERFACE;

    // NULL the output variable.
    *ppConnPt = NULL;

    //pIConnPt = pBackObj->aConnectionPoints[CONNPOINT_BALLSINK];
    if (NULL != default_connection)
    {
      IID iid;
      default_connection->GetConnectionInterface(&iid);

      if(iid == riid)
      {
        hr = default_connection->QueryInterface(IID_IConnectionPoint,
                         (void **)ppConnPt);
      }
      else
      {
        hr = CONNECT_E_NOCONNECTION;
      }
    }


  return hr;
}


/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   tLuaCOMConnPointContainer::EnumConnectionPoints

  Summary:  Return Enumerator for the connectable object's contained
            connection points.

  Args:     IEnumConnectionPoints** ppIEnum
              Pointer to the caller's Enumerator pointer variable.
              An output variable that will receive a pointer to the
              connection point enumerator COM object.

  Modifies: .

  Returns:  HRESULT
              Standard result code. NOERROR for success.
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
STDMETHODIMP tLuaCOMConnPointContainer::EnumConnectionPoints(
                       IEnumConnectionPoints** ppIEnum)
{
  HRESULT hr = NOERROR;
  IConnectionPoint* aConnPts[MAX_CONNECTION_POINTS];
  tLuaCOMEnumConnPoints* pCOEnum;


    // Zero the output interface pointer.
    *ppIEnum = NULL;

    // Make a copy on the stack of the array of connection point interfaces.
    // The copy is used below in the creation of the new Enumerator object.
//    for (i=0; i<MAX_CONNECTION_POINTS; i++)
//      aConnPts[i] = (IConnectionPoint*)pBackObj->aConnectionPoints[i];

    // Create a Connection Point enumerator COM object for the connection
    // points offered by this COBall object. Pass 'this' to be used to
    // hook the lifetime of the host object to the life time of this
    // enumerator object.
    pCOEnum = new tLuaCOMEnumConnPoints(this);
    if (NULL != pCOEnum)
    {
      // Use the array copy to Init the new Enumerator COM object.
      // Set the initial Enumerator index to 0.
      hr = pCOEnum->Init(MAX_CONNECTION_POINTS, aConnPts, 0);
      if (SUCCEEDED(hr))
      {
        // QueryInterface to return the requested interface pointer.
        // An AddRef will be conveniently done by the QI.
        if (SUCCEEDED(hr))
          hr = pCOEnum->QueryInterface(
                          IID_IEnumConnectionPoints,
                          (void **)ppIEnum);
      }
    }
    else
      hr = E_OUTOFMEMORY;


  return hr;
}



tLuaCOMConnPoint* tLuaCOMConnPointContainer::GetDefault()
{
  return default_connection;
}

void tLuaCOMConnPoint::push()
{
  LUASTACK_SET(L);

  lua_newtable(L);
  luaCompat_pushTypeByName(L, MODULENAME, LCOM_CONNPOINT_TYPENAME);
  luaCompat_setType(L, -2);

  lua_pushstring(L, CONNPOINT_NAME);
  luaCompat_pushPointer(L, this);

  lua_settable(L, -3);

  LUASTACK_CLEAN(L, 1);
}

int tLuaCOMConnPoint::tagmeth_index(lua_State *L)
{
  // pushes connection point
  lua_pushstring(L, CONNPOINT_NAME);
  lua_gettable(L, -3);

  // pushes event name
  lua_pushvalue(L, 2);

  // pushes closure to call all sinks
  lua_pushcclosure(L, call_sinks, 2);

  return 1;
}


// lua C function that dispatches events
// to sinks
int tLuaCOMConnPoint::call_sinks(lua_State *L)
{
  /// positions of parameters
  
  // self param (not used, but needed for ensuring
  // consistency
  const int self_param        = 1;

  // first user param 
  const int user_first_param  = 2;
  
  // last user param, excluding upvalues
  const int user_last_param   = luaCompat_getNumParams(L, 2);

  // upvalues
  const int connpoint         = luaCompat_upvalueIndex(L, 1, 2);
  const int event             = luaCompat_upvalueIndex(L, 2, 2);

  int num_params = 0;

  if(user_last_param < user_first_param)
    num_params = 0;
  else
    num_params = user_last_param - user_first_param + 1;

  // counter
  int i = 0;
   
  // gets connection point
  tLuaCOMConnPoint* cp = 
    (tLuaCOMConnPoint*) luaCompat_getPointer(L, connpoint);

  // call each sink
  for(i = 0; i < (int)cp->cConnections; i++)
  {
    // pushes function
    LuaBeans::push(L, cp->sinks[i]);
    cp->sinks[i]->Lock();

    lua_pushvalue(L, event);
    lua_gettable(L, -2);
    
    // removes table, leaving just function
    lua_remove(L, -2);

    // self param (mandatory but unused)
    LuaBeans::push(L, cp->sinks[i]);
    cp->sinks[i]->Lock();

    // duplicates parameters (if any)
    for(i = user_first_param; i <= user_last_param; i++)
    {
      lua_pushvalue(L, i);
    }

    // calls function (including self param)
    // ignoring errors
    luaCompat_call(L, num_params+1, 0, NULL);

    // cleans stack
    lua_settop(L, user_last_param);
  }

  // events do not return nothing
  return 0;
}

