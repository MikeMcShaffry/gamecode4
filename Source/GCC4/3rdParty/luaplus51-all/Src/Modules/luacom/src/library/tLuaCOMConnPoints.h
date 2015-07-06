#ifndef __LUACOM_CONNECTIONPOINTS_H
#define __LUACOM_CONNECTIONPOINTS_H

#include "luabeans.h"

#ifdef __cplusplus

// An enumeration giving symbol names array indexes of the connection
// points offered by the DllSndBall component in this server.
enum
{
  CONNPOINT_BALLSINK = 0
};

enum
{
  // The maximum number of connection points offered by the DllSndBall
  // component in this CONSERVE server.  The number of items in the
  // connection point enumeration above.
  MAX_CONNECTION_POINTS = 1,

  // A constant for the number of connections to add to the allocation
  // of the dynamic connection array.
  ALLOC_CONNECTIONS = 8,

  // The start value for the connection key (cookie) counter.
  COOKIE_START_VALUE = 400
};


/*O+O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O
  ObjectClass: tLuaCOMEnumConnPoints

  Summary:     COM object class for enumerating the Connection Points of
               offered by a connectable object.

  Interfaces:  IUnknown
                 Standard interface providing COM object features.
               IEnumConnectionPoints
                 Interface for connection point enumeration.

  Aggregation: tLuaCOMEnumConnPoints COM Objects are not aggregatable.
O---O---O---O---O---O---O---O---O---O---O---O---O---O---O---O---O---O---O-O*/
class tLuaCOMEnumConnPoints : public IEnumConnectionPoints
{
  public:
    // Main Object Constructor & Destructor.
    tLuaCOMEnumConnPoints(IUnknown* pHostObj);
    ~tLuaCOMEnumConnPoints(void);

    // A general method for initializing this newly created object.
    // Creates any subordinate arrays, structures, or objects.
    HRESULT Init(
              ULONG cConnPts,
              IConnectionPoint** paConnPts,
              ULONG iEnumIndex);

    // IUnknown methods. Main object, non-delegating.
    STDMETHODIMP         QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // IEnumConnectionPoints methods.
    STDMETHODIMP         Next(ULONG, IConnectionPoint**, ULONG*);
    STDMETHODIMP         Skip(ULONG);
    STDMETHODIMP         Reset(void);
    STDMETHODIMP         Clone(IEnumConnectionPoints**);

  private:
    // Private data of tLuaCOMEnumConnPoints COM objects.

    // Main Object reference count.
    ULONG              cRefs;

    // IUnknown pointer to host COM object being enumerated.
    IUnknown*          pHostObj;

    // Connection Point index variable.
    ULONG              iEnumIndex;

    // Number of Connection Points being enumerated.
    ULONG              cConnPts;

    // Allocated array of Connection Point interface pointers.
    IConnectionPoint** paConnPts;
};

typedef tLuaCOMEnumConnPoints* PtLuaCOMEnumConnPoints;


/*O+O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O
  ObjectClass: tLuaCOMConnPoint

  Summary:     Connection Point COM object class. Implements a native
               IConnectionPoint interface. The Advise, Unadvise, and
               EnumConnections methods use the CThreaded OwnThis mechanism
               to provide thread-safe mutually exclusive access to this
               connection point object.

  Interfaces:  IUnknown
                 Standard interface providing COM object features.
               IConnectionPoint
                 Interface for connection point features.

  Aggregation: tLuaCOMConnPoint COM Objects are not aggregatable.
O---O---O---O---O---O---O---O---O---O---O---O---O---O---O---O---O---O---O-O*/
class tLuaCOMConnPoint : public IConnectionPoint
{
  public:
	  static int tagmeth_index(lua_State* L);
	  void push(void);
    // Main Object Constructor & Destructor.
    tLuaCOMConnPoint(lua_State* L, IUnknown* pHostObj);
    ~tLuaCOMConnPoint(void);

    // A general method for initializing this newly created object.
    // Creates any subordinate arrays, structures, or objects.
    HRESULT Init(REFIID riid, ITypeInfo *typeinfo);

    // IUnknown methods. Main object, non-delegating.
    STDMETHODIMP         QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // IConnectionPoint methods.
    STDMETHODIMP GetConnectionInterface(IID*);
    STDMETHODIMP GetConnectionPointContainer(IConnectionPointContainer**);
    STDMETHODIMP Advise(IUnknown*, DWORD*);
    STDMETHODIMP Unadvise(DWORD);
    STDMETHODIMP EnumConnections(IEnumConnections**);

  private:
    // Private utility methods of tLuaCOMConnPoint.
    HRESULT GetSlot(UINT* puiFreeSlot);
    HRESULT FindSlot(DWORD dwCookie, UINT* puiSlot);

    // Private data of tLuaCOMConnPoint COM objects.

    // Main Object reference count.
    ULONG          cRefs;

    // IUnknown pointer to host COM object offering this connection point.
    IUnknown*      pHostObj;

    // The IID of the sink interface associated with this connection point.
    IID            iidSink;

    // The current connection cookie (key) counter.
    DWORD          dwNextCookie;

    // The current number of live sink connections to this connection point.
    UINT           cConnections;

    // The current maximum index into the dynamic connection array.
    UINT           uiMaxIndex;

    // The dynamic array of sink connections to this connection point.
    CONNECTDATA   paConnections[ALLOC_CONNECTIONS];

    // holds LuaCOM objects for each sink
    tLuaCOM* sinks[ALLOC_CONNECTIONS];
protected:
	static int call_sinks(lua_State *L);
	ITypeInfo* source_typeinfo;
  lua_State* L;
};

typedef tLuaCOMConnPoint* PtLuaCOMConnPoint;


/*O+O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O+++O
  ObjectClass: tLuaCOMEnumConnections

  Summary:     COM object class for enumerating the connections of
               a connection point of a connectable object.

  Interfaces:  IUnknown
                 Standard interface providing COM object features.
               IEnumConnections
                 Interface for connection enumeration features.

  Aggregation: tLuaCOMEnumConnections COM Objects are not aggregatable.
O---O---O---O---O---O---O---O---O---O---O---O---O---O---O---O---O---O---O-O*/
class tLuaCOMEnumConnections : public IEnumConnections
{
  public:
    // Main Object Constructor & Destructor.
    tLuaCOMEnumConnections(IUnknown* pHostObj);
    ~tLuaCOMEnumConnections(void);

    // A general method for initializing this newly created object.
    // Creates any subordinate arrays, structures, or objects.
    HRESULT Init(
              ULONG cConnections,
              CONNECTDATA* paConnections,
              ULONG iEnumIndex);

    // IUnknown methods. Main object, non-delegating.
    STDMETHODIMP         QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // IEnumConnections methods.
    STDMETHODIMP         Next(ULONG, CONNECTDATA*, ULONG*);
    STDMETHODIMP         Skip(ULONG);
    STDMETHODIMP         Reset(void);
    STDMETHODIMP         Clone(IEnumConnections**);

  private:
    // Private data of tLuaCOMEnumConnections COM objects.

    // Main Object reference count.
    ULONG            cRefs;

    // IUnknown pointer to host connection point COM object being
    // enumerated.
    IUnknown*        pHostObj;

    // Connection index variable.
    ULONG            iEnumIndex;

    // Number of Connections being enumerated.
    ULONG            cConnections;

    // Allocated array of live Connections only.
    CONNECTDATA*     paConnections;
};

typedef tLuaCOMEnumConnections* PtLuaCOMEnumConnections;



class tLuaCOMConnPointContainer : public IConnectionPointContainer
{
  public:
	  tLuaCOMConnPoint* GetDefault(void);
    tLuaCOMConnPointContainer(lua_State *p_L, IUnknown* p_pUnkOuter);

    // Interface Implementation Constructor & Destructor.
    ~tLuaCOMConnPointContainer(void);

    // IUnknown methods.
    STDMETHODIMP         QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // IConnectionPointContainer methods.
    STDMETHODIMP         FindConnectionPoint(REFIID, IConnectionPoint**);
    STDMETHODIMP         EnumConnectionPoints(IEnumConnectionPoints**);

  private:
    // Data private to this interface implementation.
    IUnknown*     pUnkOuter;    // Outer unknown for Delegation.
protected:
	long num_connection_points;
	long max_connection_points;
	tLuaCOMConnPoint* default_connection;
	tLuaCOMConnPoint** connection_points;
  lua_State* L;
};



#endif // __cplusplus


#endif // CONNECT_H

















