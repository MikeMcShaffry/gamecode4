// WatchCtl.cpp : Implementation of CWatchCtl
#include "stdafx.h"
#include "WatchCtl.h"

#pragma warning(disable: 4996)

CWatchCtl* g_watchCtl;
HANDLE g_processHandle;

class DebuggerEventsSink : public IDispEventImpl<1, DebuggerEventsSink, &__uuidof(EnvDTE::_dispDebuggerEvents), &EnvDTE::LIBID_EnvDTE, 7, 0>
{
public:
	BEGIN_SINK_MAP(DebuggerEventsSink)
		SINK_ENTRY_EX(1, __uuidof(EnvDTE::_dispDebuggerEvents), 1, OnEnterRunMode)
		SINK_ENTRY_EX(1, __uuidof(EnvDTE::_dispDebuggerEvents), 1, OnEnterDesignMode)
	END_SINK_MAP()

	HRESULT __stdcall OnEnterRunMode(EnvDTE::dbgEventReason Reason)
	{
		return S_OK;
	}

	HRESULT __stdcall OnEnterDesignMode(EnvDTE::dbgEventReason Reason)
	{
		return S_OK;
	}
};


DebuggerEventsSink m_debuggerEventsSink;
CComPtr<EnvDTE::_DebuggerEvents> m_debuggerEvents;


STDMETHODIMP CWatchCtl::OnConnect(IDispatch* pApplication)
{
	g_watchCtl = this;
	pApplication->QueryInterface(__uuidof(EnvDTE::_DTE), (LPVOID*)&m_pDTE);

	// Retrieve the list of events.
	CComPtr<EnvDTE::Events> pEvents;
	HRESULT hr = m_pDTE->get_Events( &pEvents );
	if ( FAILED( hr ) )
		return hr;

	// If there are events registered and we got here, then VStudio is
	// really messing with us...
	if (m_debuggerEvents)
	{
		m_debuggerEventsSink.DispEventUnadvise((IUnknown*)m_debuggerEvents.p);
		m_debuggerEvents = NULL;
	}

	// Register the event systems.
	if(SUCCEEDED(pEvents->get_DebuggerEvents((EnvDTE::_DebuggerEvents**)&m_debuggerEvents)))
	{
		m_debuggerEventsSink.DispEventAdvise((IUnknown*)m_debuggerEvents.p);
	}

	return S_OK;
}


STDMETHODIMP CWatchCtl::OnDisconnect()
{
	// If there are events registered and we got here, then VStudio is
	// really messing with us...
	if (m_debuggerEvents)
	{
		m_debuggerEventsSink.DispEventUnadvise((IUnknown*)m_debuggerEvents.p);
		m_debuggerEvents = NULL;
	}

	m_pDTE = NULL;

	return S_OK;
}


LRESULT CWatchCtl::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CWatchCtl* pT = static_cast<CWatchCtl*>(this);
	RECT r;
	bHandled = false;
	pT->GetClientRect(&r);
	_minClientSize.cx = (r.right-r.left);
	_minClientSize.cy = (r.bottom-r.top);
	_prevClientSize = _minClientSize;
	pT->GetWindowRect(&r);
	_minWindowSize.cx = (r.right-r.left);
	_minWindowSize.cy = (r.bottom-r.top);
	pT->DefineLayout();
	pT->SetScrollSize(_minClientSize);
	bHandled = false;

	m_luaStateNameWnd.Attach(GetDlgItem(IDC_VD_LUASTATEVARIABLENAME));
	m_evaluateSymbolRadio.Attach(GetDlgItem(IDC_EVALUATESYMBOL));
	m_globalsRadio.Attach(GetDlgItem(IDC_GLOBALS));
	m_registryRadio.Attach(GetDlgItem(IDC_REGISTRY));
	m_localsRadio.Attach(GetDlgItem(IDC_LOCALS));
	m_xbox360Check.Attach(GetDlgItem(IDC_XBOX360));
	m_symbolWnd.Attach(GetDlgItem(IDC_SYMBOL));
	m_tableTree.Attach(GetDlgItem(IDC_TABLES));
	m_keyValueList.Attach(GetDlgItem(IDC_KEYVALUES));

	m_keyValueList.InsertColumn(0, _T("Key"), LVCFMT_LEFT, 100, -1);
	m_keyValueList.InsertColumn(1, _T("Value"), LVCFMT_LEFT, 200, -1);
	m_keyValueList.InsertColumn(2, _T("Type"), LVCFMT_LEFT, 50, -1);

	m_globalsRadio.SetCheck(1);

	AttachForm(IDC_VD_ACCEPTLUASTATENAME, ATTACH_RIGHT);
	AttachForm(IDC_VD_ACCEPTLUASTATENAME, ATTACH_TOP);
	AttachForm(IDC_KEYVALUES, ATTACH_LEFT);
	AttachForm(IDC_KEYVALUES, ATTACH_TOP);
	AttachForm(IDC_KEYVALUES, ATTACH_RIGHT);
	AttachForm(IDC_KEYVALUES, ATTACH_BOTTOM);
	AttachForm(IDC_TABLES, ATTACH_TOP);
	AttachForm(IDC_TABLES, ATTACH_BOTTOM);
/*	AttachForm(IDC_IMG, ATTACH_HCENTER);
	AttachForm(IDC_IMG, ATTACH_VCENTER);
	AttachForm(IDC_LABEL, ATTACH_HCENTER);
	AttachForm(IDC_LABEL, ATTACH_VCENTER);
*/
/*	CRect rcVert;
	GetClientRect(&rcVert);

	// create the vertical splitter
	m_splitterWnd.Create(m_hWnd, rcVert, NULL,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

	m_splitterWnd.SetSplitterPane(SPLIT_PANE_LEFT, m_tableTree.m_hWnd);
	m_splitterWnd.SetSplitterPane(SPLIT_PANE_RIGHT, m_keyValueList.m_hWnd);
*/
	return 0;
}


void CWatchCtl::Initialize()
{
	m_LuaStackObjectPtr = NULL;
	m_LuaObjectPtr = NULL;
	m_LuaStateOwnerPtr = NULL;
	m_LuaStatePtr = NULL;
	m_luaStatePtr = NULL;
	m_lookupObject = NULL;
}


















bool m_isXbox360;

// memory
typedef HRESULT (*fnDmGetMemory)(LPCVOID lpbAddr, DWORD cb, LPVOID lpbBuf,
    LPDWORD pcbRet);

fnDmGetMemory DmGetMemory;
HMODULE m_hModule;


void ReadMem(void* pHelper, const void* fromPtr, void* toPtr, size_t size)
{
	if (m_isXbox360)
	{
		if (!DmGetMemory)
		{
			CRegKey devKey;
			if (devKey.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Classes\\CLSID\\{a5eb45d8-f3b6-49b9-984a-0d313ab60342}\\InprocServer32")) != ERROR_SUCCESS)
				throw -1;

			ULONG nChars = 1000;
			TCHAR buffer[1000];
			if (devKey.QueryStringValue(_T(""), buffer, &nChars) != ERROR_SUCCESS)
				throw -1;

			CString key = buffer;
			int slashPos = key.ReverseFind('\\');
			if (slashPos == -1)
				throw -1;

			key = key.Left(slashPos + 1);

			m_hModule = LoadLibrary(key + _T("xbdm.dll"));
			if (m_hModule == NULL)
				throw -1;

			DmGetMemory = (fnDmGetMemory)GetProcAddress(m_hModule, "_DmGetMemory@16");
			if (!DmGetMemory)
				throw -1;
		}

		DWORD numRead;
		HRESULT hr = DmGetMemory(fromPtr, size, toPtr, &numRead);
		if (FAILED(hr))
		{
			throw -1;
		}

		if (size != numRead)
			throw -1;
	}
	else
	{
		SIZE_T numRead;
		if (!ReadProcessMemory(g_processHandle, fromPtr, toPtr,
				size, &numRead))
		{
			throw -1;
		}

		if (size != numRead)
			throw -1;
	}
}


int GetTableCount(TValue& obj)
{
//	StkId t;
//	const TObject *value;
	int n;
	Node *nd;
	Table *a = hvalue(&obj);
	Table aBuf;
	ReadMem_Table(NULL, a, aBuf);
	lua_Number max = 0;
	int i;
	i = aBuf.sizearray;
	while (i--)
	{
		TValue obj;
		ReadMem_TValue(NULL, &aBuf.array[i], obj);
		if (ttype(&obj) != LUA_TNIL)
			break;
	}
	max = i+1;
	i = sizenode(&aBuf);

	Node ndBuf;
	nd = aBuf.node;
	ReadMem_Node(NULL, nd, ndBuf);
	while (i--) {
		if (ttype(gkey(&ndBuf)) != LUA_TNIL)
			max++;
		nd++;
		ReadMem_Node(NULL, nd, ndBuf);
	}
	n = cast(int, max);
	return n;
}


CStringW FormatObject(TValue& obj, CString& type)
{
	CStringW out;

	switch (ttype(&obj))
	{
		case LUA_TNONE:
			out = _T("NONE");
			type = _T("None");
			break;

		case LUA_TNIL:
			out = _T("NIL");
			type = _T("Nil");
			break;

		case LUA_TBOOLEAN:
			out = bvalue(&obj) ? _T("true") : _T("false");
			type = "Boolean";
			break;

		case LUA_TLIGHTUSERDATA:
			out.Format(L"0x%08x", pvalue(&obj));
			type = "LightUserData";
			break;

		case LUA_TNUMBER:
			out.Format(L"%.14g", nvalue(&obj));
			type = "Number";
			break;

		case LUA_TSTRING:
		{
			TString data;
			ReadMem_TString(NULL, tsvalue(&obj), data);

			CStringA temp;
			LPSTR buf = temp.GetBufferSetLength(data.tsv.len);
			ReadMem(NULL, (TString*)(tsvalue(&obj)) + 1, buf, data.tsv.len);
			temp.ReleaseBuffer(data.tsv.len);

			out = L"\"" + CStringW(temp) + L"\"";
			type = "String";
			break;
		}

		case LUA_TTABLE:
		{
			int tableCount = GetTableCount(obj);
			out.Format(L"count=%d", tableCount);
			type = "Table";
			break;
		}

		case LUA_TFUNCTION:
		{
			Closure cl;
			ReadMem_Closure(NULL, gcvalue(&obj), cl);

			out = cl.c.isC ? "CFUNCTION" : "FUNCTION";
			type = "Function";
			break;
		}

		case LUA_TUSERDATA:
		{
			Udata data;
			ReadMem(NULL, gcvalue(&obj), &data, sizeof(Udata));
			SwapEndian(data.uv.len);

			void* ptr;
			ptr = ((Udata*)obj.value.gc) + 1;
			if ( data.uv.len & 4 )
			{
				// The user data pointer is embedded.
				ReadMem( NULL, ptr, &ptr, sizeof(void*) );
			}

			out.Format(L"0x%08x", ptr);
			type = "Userdata";
			break;
		}

		case LUA_TTHREAD:
			out = L"[THREAD]";
			break;

		case LUA_TWSTRING:
		{
			TString data;

			ReadMem_TString(NULL, obj.value.gc, data);

			LPWSTR buf = out.GetBufferSetLength(data.tsv.len);
			ReadMem(NULL, (void*)( ((TString*)obj.value.gc) + 1 ), buf, data.tsv.len * 2);
			if (g_swapEndian)
			{
				for (size_t i = 0; i < data.tsv.len; ++i)
					SwapEndian(buf[i]);
			}
			out.ReleaseBuffer(data.tsv.len);
			type = "WString";
			break;
		}
	}

	return out;
}


CStringW FormatObject(TValue* o, CString& type)
{
	TValue obj;
	ReadMem_TValue(NULL, o, obj);
	return FormatObject(obj, type);
}



#if 0
bool lua_getlocal(lua_State *L, const lua_Debug *ar, int n, CStringA& name, TValue*& value)
{
	CallInfo *ci;
	Proto *fp;
	CallInfo ciBuf;

	name.Empty();

	ci = L->base_ci + ar->_ci;
	ReadMem(NULL, ci, &ciBuf, sizeof(CallInfo));

	fp = getluaproto(ci);
	if (fp)
	{  /* is a Lua function? */
		bool ret = luaF_getlocalname(fp, n, currentpc(L, ci), name);
		if (ret)
		{
//			value = FormatObject(ciBuf.base + (n - 1), type);
			value = ciBuf.base + (n - 1);
		}
		//    if (name)
		//      luaA_pushobject(L, ci->base+(n-1));  /* push value */
		return ret;
	}

	return false;
}
#endif

static TString *newlstr (lua_State *L, const char *str, size_t l, unsigned int h) {
  TString *ts;
  ts = cast(TString *, malloc((l+1)*sizeof(char)+sizeof(TString)));
  ts->tsv.len = l;
  ts->tsv.hash = h;
  ts->tsv.marked = 0; //jj luaC_white(G(L));
  ts->tsv.tt = LUA_TSTRING;
  ts->tsv.reserved = 0;
  memcpy(ts+1, str, l*sizeof(char));
  ((char *)(ts+1))[l] = '\0';  /* ending 0 */
  return ts;
}


TString *luaS_newlstr (lua_State *L, const char *str, size_t l) {
  unsigned int h = (unsigned int)l;  /* seed */
  size_t step = (l>>5)+1;  /* if string is too long, don't hash all its chars */
  size_t l1;
  for (l1=l; l1>=step; l1-=step)  /* compute hash */
    h = h ^ ((h<<5)+(h>>2)+(unsigned char)(str[l1-1]));
  return newlstr(L, str, l, h);  /* not found */
}


/*
** max size of array part is 2^MAXBITS
*/
#define BITS_INT	32
#if BITS_INT > 26
#define MAXBITS		24
#else
#define MAXBITS		(BITS_INT-2)
#endif

/* check whether `x' < 2^MAXBITS */
#define toobig(x)	((((x)-1) >> MAXBITS) != 0)

/*
** returns the index for `key' if `key' is an appropriate key to live in
** the array part of the table, -1 otherwise.
*/
static int arrayindex (const TValue *key)
{
  if (ttisnumber(key))
  {
    lua_Number n = nvalue(key);
    int k;
    lua_number2int(k, n);
    if (luai_numeq(cast_num(k), n))
      return k;
  }
  return -1;  /* `key' did not match some condition */
}


/*
** returns the index of a `key' for table traversals. First goes all
** elements in the array part, then elements in the hash part. The
** beginning of a traversal is signalled by -1.
*/
static int luaH_findindex (lua_State *L, Table *t, TValue *key,
						bool localKey, bool localString) {
	Table tBuf;
	ReadMem_Table(NULL, t, tBuf);
	t = &tBuf;

	if (!localKey)
	{
		ReadMem_TValue(NULL, key, *key);
	}

	int i;
	if (ttisnil(key)) return -1;  /* first iteration */
	i = arrayindex(key);
	if (0 < i && i <= tBuf.sizearray) {  /* is `key' inside array part? */
		return i-1;  /* yes; that's the index (corrected to C) */
	}
	else
	{
	    Node *n = mainposition(NULL, t, key);
		Node nBuf;
		do {  /* check whether `key' is somewhere in the chain */
			/* key may be dead already, but it is ok to use it in `next' */
			ReadMem_Node(NULL, n, nBuf);
			if (luaO_rawequalObj(key2tval(&nBuf), key) ||
				(ttype(gkey(&nBuf)) == LUA_TDEADKEY && iscollectable(key) &&
				gcvalue(gkey(&nBuf)) == gcvalue(key))) {
					i = cast_int(n - gnode(&tBuf, 0));  /* key index in hash table */
					/* hash elements are numbered after array ones */
					return i + tBuf.sizearray;
			}
			else n = gnext(&nBuf);
		} while (n);

		return 0;
	}
}


int luaH_next (lua_State *L, Table *t, TValue *key, bool localKey,
			   TValue& outValue)
{
	int i = luaH_findindex(L, t, key, localKey, false);  /* find original element */
	Table tBuf;
	ReadMem_Table(NULL, t, tBuf);
	for (i++; i < tBuf.sizearray; i++) {  /* try first array part */
		TValue arrayObj;
		ReadMem_TValue(NULL, &tBuf.array[i], arrayObj);
		if (!ttisnil(&arrayObj)) {  /* a non-nil value? */
			setnvalue(key, cast_num(i+1));
			setobj2s(L, &outValue, &arrayObj);
			return 1;
		}
	}
	for (i -= tBuf.sizearray; i < sizenode(&tBuf); i++)
	{  /* then hash part */
		Node nodeBuf;
		ReadMem_Node(NULL, &tBuf.node[i], nodeBuf);
		if (!ttisnil(gval(&nodeBuf)) != LUA_TNIL) {  /* a non-nil value? */
			setobj2s(L, key, key2tval(&nodeBuf));
			setobj2s(L, &outValue, gval(&nodeBuf));
			return 1;
		}
	}
	return 0;  /* no more elements */
}


void CWatchCtl::OnPlusExpand(CTreeItem treeItem)
{
	// Let's see if it's already filled in.
	CTreeItem childItem = treeItem.GetChild();
	if (childItem)
	{
		CString childText;
		childItem.GetText(childText);
		if (childText != _T("***Expanding***"))
			return;
	}

	childItem.Delete();

	TValue key2Obj;
	key2Obj.tt = LUA_TNIL;
	bool isLocalKey = true;

	CTreeItem curTableItem = treeItem;

	TValue table = *(TValue*)treeItem.GetData();
    bool checkedmeta = false;
	while (true)
	{
		TValue outValue;
		if (!checkedmeta && !luaH_next(&theState, hvalue(&table), &key2Obj, isLocalKey, outValue))
        {
            if (!getmetatable(NULL, &table) || getmetatable(NULL, &table) == (void*) 0xfeeefeee)// || getmetatable((&tableObj) == tableObj.value.gc->h))
                break;

            checkedmeta = true;

            acquiremetatable(getmetatable(NULL, &table), &outValue);
        }
        else if (checkedmeta)
            break;

		if (outValue.tt == LUA_TTABLE)
		{
			CString type;
			CStringW keyStr;
            if (!checkedmeta)
                keyStr = FormatObject(key2Obj, type);
            else
                keyStr = L"metatable";

			CTreeItem treeItem = curTableItem.AddHead(keyStr, -1);
			TValue* newValue = new TValue;
			*newValue = outValue;
			treeItem.SetData((DWORD)newValue);
			treeItem.AddHead(_T("***Expanding***"), -1);
		}
	}
	treeItem.SortChildren();
}


void CWatchCtl::FillInKeyValues(void* inTableObj, CTreeItem* treeItem)
{
	TValue tableObj = *(TValue*)inTableObj;
	m_keyValueList.DeleteAllItems();

	CStringW value;
	CString type;
	if (tableObj.tt != LUA_TTABLE)
	{
		value = FormatObject(tableObj, type);
		int item = m_keyValueList.InsertItem(m_keyValueList.GetItemCount() + 1, _T(""));
		m_keyValueList.SetItemText(item, 1, value);
		m_keyValueList.SetItemText(item, 2, type);
	}
	else
	{
		TValue key2Obj;
		key2Obj.tt = LUA_TNIL;
		bool isLocalKey = true;

		CTreeItem curTableItem;
		if (treeItem)
			curTableItem = *treeItem;

        bool checkedmeta = false;

		while (true)
		{
			TValue outValue;
			if (!checkedmeta && !luaH_next(&theState, hvalue(&tableObj), &key2Obj, isLocalKey, outValue))
            {
                if (!getmetatable(NULL, &tableObj) || getmetatable(NULL, &tableObj) == (void*) 0xfeeefeee)// || getmetatable((&tableObj) == tableObj.value.gc->h))
                    break;

                checkedmeta = true;

                acquiremetatable(getmetatable(NULL, &tableObj), &outValue);
            }
            else if (checkedmeta)
                break;

			CStringW keyStr;
            if (!checkedmeta)
                keyStr = FormatObject(key2Obj, type);
            else
                keyStr = L"metatable";

			if (outValue.tt == LUA_TTABLE)
			{
				if (treeItem)
				{
					CTreeItem tableTreeItem = curTableItem.AddHead(keyStr, -1);
					TValue* newValue = new TValue;
					*newValue = outValue;
					tableTreeItem.SetData((DWORD)newValue);
					tableTreeItem.AddHead(_T("***Expanding***"), -1);
				}
			}

			CStringW value = FormatObject(outValue, type);

			int item = m_keyValueList.InsertItem(m_keyValueList.GetItemCount() + 1, keyStr);
			m_keyValueList.SetItemText(item, 1, value);
			m_keyValueList.SetItemText(item, 2, type);
		}

		if (treeItem)
			curTableItem.SortChildren();
	}


}


void CWatchCtl::Refresh()
{
	try
	{
		CComPtr<EnvDTE::Debugger> pDebugger;
		if (FAILED(m_pDTE->get_Debugger(&pDebugger)))
		{
			return;
		}

		CComPtr<EnvDTE::Process> pProcess;
		if (FAILED(pDebugger->get_CurrentProcess(&pProcess)))
		{
			return;
		}

		if (!pProcess)
		{
			return;
		}

		long processID;
		if (FAILED(pProcess->get_ProcessID(&processID)))
		{
			return;
		}

		m_pDebugger = pDebugger;
		m_processID = processID;

		m_isXbox360 = m_xbox360Check.GetCheck() != 0;
		g_swapEndian = m_isXbox360 ? true : false;

		ReadAddresses();

		lua_State* L = &theState;

		TValue m_lookupObject;
		m_lookupObject = L->l_gt;

		m_tableTree.DeleteAllItems();
		m_keyValueList.DeleteAllItems();

		if (m_evaluateSymbolRadio.GetCheck() != 0)
		{
			if (m_LuaObjectPtr)
			{
				LuaObject luaObject;
				ReadMem_LuaObject(NULL, m_LuaObjectPtr, luaObject);
				m_lookupObject = luaObject.m_object;
			}

			if (!m_symbol.IsEmpty())
			{
				m_lookupObject = LookupSymbol(m_lookupObject, m_symbol);
			}
		}
		else if (m_globalsRadio.GetCheck() != 0)
		{
			StkId globals = &L->l_gt;
			m_lookupObject = *globals;
		}
		else if (m_registryRadio.GetCheck() != 0)
		{
            global_State gs;
            ReadMem(NULL, L->l_G, &gs, sizeof(global_State));
//			SwapEndian(gs._registry);
			StkId registryPtr = &gs.l_registry;
			m_lookupObject = *registryPtr;
		}
		else if (m_localsRadio.GetCheck() != 0)
		{
#if 0
			// Read locals.
			lua_Debug debugInfo;
			if (lua_getstack(L, 0, &debugInfo) != 0)
			{
				lua_getinfo(L, "Sl", &debugInfo);

				// Wander the list of local variables.
				int i = 1;
				CString localName;
				CTreeItem curTableItem = m_tableTree.GetRootItem();
				TObject* localValue;
				while (lua_getlocal(L, &debugInfo, i++, localName, localValue))
				{
					TObject localValueObj;
					ReadMem(localValue, &localValueObj, sizeof(TObject));
					FillInKeyValues(&localValueObj, &curTableItem);
	//				lua_pop(L, 1);  // remove variable value
				}
			}
//			StkId registryPtr = registry(L);
//			m_lookupObject = *registryPtr;
#endif 0
			return;
		}

		if (!ttisnil(&m_lookupObject))
		{
			CTreeItem curTableItem = m_tableTree.GetRootItem().AddHead(_T("Object"), -1);
			TValue* newObj = new TValue;
			*newObj = m_lookupObject;
			curTableItem.SetData((DWORD)newObj);

			FillInKeyValues(&m_lookupObject, &curTableItem);
		}
	}
	catch (...)
	{
	}
}


#include "Lex.h"

TValue CWatchCtl::LookupSymbol(TValue obj, CString symbol)
{
	m_tableTree.DeleteAllItems();
	m_keyValueList.DeleteAllItems();

	TValue nilObj;
	nilObj.tt = LUA_TNIL;

	Lex lex;
	lex.Init(symbol);

	while (!lex.Eof())
	{
		if (obj.tt != LUA_TTABLE)
			return nilObj;

		LexToken token;
		lex.GetToken(token);
		if (token.IsInteger())
		{
			// Lookup number
			const TValue* v = luaH_getnum (NULL, hvalue(&obj), token.GetInteger());
			if (!v)
				return nilObj;

			ReadMem_TValue(NULL, (void*)v, obj);

			// Continue trying.
		}
		else if (token.IsString()  ||  token.IsIdent())
		{
			CStringA str = CStringA(token.IsString() ? token.GetString() : token.GetIdent());
			// Lookup string
			TString* strObj = luaS_newlstr(&theState, str, str.GetLength());
//			TObject keyObj;
//			keyObj.tt = LUA_TSTRING;
//			keyObj.value.ts = strObj;

			const TValue* v = luaH_getstr(NULL, hvalue(&obj), strObj, true);
			free(strObj);

			if (!v)
				return nilObj;

			ReadMem_TValue(NULL, (void*)v, obj);
		}
		else
		{
			return nilObj;
			assert(0);
		}

		lex.GetToken(token);
		if (token.GetType() != '.')
			break;
	}

	return obj;
}


void CWatchCtl::UpdateLuaObjectAddress(void)
{
	Refresh();

	lua_State* L = &theState;

/*	StkId globals = gt(L);

	CStringA varName = "Table";
	TString* strObj = luaS_newlstr(&state, "Table", 5);
	TObject keyObj;
	keyObj.tt = LUA_TSTRING;
	keyObj.value.ts = strObj;
//	keyObj.tt = LUA_TNUMBER;
//	keyObj.value.n = 1;

	TObject resultObj;
	resultObj = luaV_gettable(&state, globals, &keyObj, 0, true);

//	TObject resultObjBuf;
//	ReadMem(resultObj, &resultObjBuf, sizeof(TObject));
*/

//	CWindow editField = GetDlgItem(IDC_OUTPUT);
//	editField.SetWindowText(str);
//	OutputDebugString(str);


#if 0
	// Read locals.
	lua_Debug debugInfo;
	if (lua_getstack(&state, 0, &debugInfo) != 0)
	{
		lua_getinfo(&state, "Sl", &debugInfo);

		// Wander the list of local variables.
		int i = 1;
		CStringA localName;
		CStringA localValue;
		while (lua_getlocal(&state, &debugInfo, i++, localName, localValue))
		{
			OutputDebugString(localName);
			OutputDebugString("\t\t");
			OutputDebugString(localValue);
			OutputDebugString("\n");
/*			// If the name matches the local variable, then choose it.
			if (findName == localName)
			{
				break;
			}

			lua_pop(L, 1);  // remove variable value
*/
		}
	}
#endif 0
}



void CWatchCtl::UpdateLuaStateAddress(void)
{
#if 0
	lua_State* L = &state;
	StkId globals = gt(L);
#if 0
	CStringA varName = "Table";
	TString* strObj = luaS_newlstr(&state, "Table", 5);
	TObject keyObj;
	keyObj.tt = LUA_TSTRING;
	keyObj.value.ts = strObj;
//	keyObj.tt = LUA_TNUMBER;
//	keyObj.value.n = 1;

	TObject resultObj;
	resultObj = luaV_gettable(&state, globals, &keyObj, 0, true);

//	TObject resultObjBuf;
//	ReadMem(resultObj, &resultObjBuf, sizeof(TObject));

	TObject key2Obj;
	key2Obj.tt = LUA_TNIL;
	bool isLocalKey = true;

	while (true)
	{
		TObject outValue;
		if (!luaH_next(&state, hvalue(&resultObj), &key2Obj, isLocalKey, outValue))
			break;

		isLocalKey = false;

		OutputDebugString(FormatObject(outValue));
		OutputDebugString("\n");
	}

	free(keyObj.value.ts);
#endif 0


#if 0
	// Read locals.
	lua_Debug debugInfo;
	if (lua_getstack(&state, 0, &debugInfo) != 0)
	{
		lua_getinfo(&state, "Sl", &debugInfo);

		// Wander the list of local variables.
		int i = 1;
		CStringA localName;
		CStringA localValue;
		while (lua_getlocal(&state, &debugInfo, i++, localName, localValue))
		{
			OutputDebugString(localName);
			OutputDebugString("\t\t");
			OutputDebugString(localValue);
			OutputDebugString("\n");
/*			// If the name matches the local variable, then choose it.
			if (findName == localName)
			{
				break;
			}

			lua_pop(L, 1);  // remove variable value
*/
		}
	}
#endif 0

#endif 0
}

bool m_isHex;

void CWatchCtl::ReadAddresses()
{
	m_LuaStackObjectPtr = NULL;
	m_LuaObjectPtr = NULL;
	m_LuaStateOwnerPtr = NULL;
	m_LuaStatePtr = NULL;
	m_luaStatePtr = NULL;

	m_symbolWnd.GetWindowText(m_symbol);

	CString varName;
	m_luaStateNameWnd.GetWindowText(varName);
	if (varName.IsEmpty())
	{
		m_luaStatePtr = NULL;
	}
	else
	{
		if (varName.Left(2) == "0x")
		{
			_tscanf(varName, "0x%x", &m_luaStatePtr);
			m_LuaStatePtr = NULL;
		}
		else
		{
			CComPtr<EnvDTE::Expression> pExpression;
			if (SUCCEEDED(m_pDebugger->GetExpression(CComBSTR(varName), VARIANT_FALSE, -1, &pExpression)))
			{
				VARIANT_BOOL isValidValue;
				pExpression->get_IsValidValue(&isValidValue);

				if (isValidValue == VARIANT_TRUE)
				{
					CComBSTR bstrType;
					pExpression->get_Type(&bstrType);
					CStringA strType(bstrType);

					if (strType[strType.GetLength() - 1] != '*') //  &&  strType[strType.GetLength() - 1] != '&')
					{
						varName = _T("&") + varName;
					}
				}
			}

			m_isHex = true;
			CComPtr<EnvDTE::Expression> pExpression2;
			if (SUCCEEDED(m_pDebugger->GetExpression(CComBSTR(varName), VARIANT_FALSE, -1, &pExpression2)))
			{
				VARIANT_BOOL isValidValue;
				pExpression2->get_IsValidValue(&isValidValue);

				CStringA strType;

				if (isValidValue == VARIANT_TRUE)
				{
					CComBSTR bstrType;
					pExpression2->get_Type(&bstrType);
					strType = bstrType;

					if (strType.Find("ManagedLuaPlus.LuaObject") != -1)
					{
						pExpression2 = NULL;
						m_isHex = false;

						if (SUCCEEDED(m_pDebugger->GetExpression(CComBSTR(_T("*") + varName + _T(".m_object")), VARIANT_FALSE, -1, &pExpression2)))
						{
							pExpression2->get_IsValidValue(&isValidValue);

							strType = "LuaPlus::LuaObject";
						}
						else
						{
							isValidValue = VARIANT_FALSE;
						}
					}
					else if (strType.Find("ManagedLuaPlus.LuaState") != -1)
					{
						pExpression2 = NULL;
						m_isHex = false;

						if (SUCCEEDED(m_pDebugger->GetExpression(CComBSTR(_T("*") + varName + _T(".m_state")), VARIANT_FALSE, -1, &pExpression2)))
						{
							pExpression2->get_IsValidValue(&isValidValue);
							strType = "LuaPlus::LuaState";
						}
						else
						{
							isValidValue = VARIANT_FALSE;
						}
					}
				}

				if (isValidValue == VARIANT_TRUE)
				{
					CComBSTR bstrValue;
					pExpression2->get_Value(&bstrValue);

					void* address;
					CStringA strValue(bstrValue);
					if (m_isHex)
						sscanf(strValue, "0x%x", &address);
					else
						sscanf(strValue, "%u", &address);

					if (strType.Find("LuaPlus::LuaStateOwner") != -1)
					{
						m_LuaStateOwnerPtr = address;
					}
					else if (strType.Find("LuaPlus::LuaState") != -1)
					{
						m_LuaStateOwnerPtr = NULL;
						m_LuaStatePtr = address;
					}
					else if (strType.Find("LuaPlus::LuaObject") != -1)
					{
						m_LuaObjectPtr = address;
						m_evaluateSymbolRadio.SetCheck(1);
					}
					else if (strType.Find("LuaPlus::LuaStackObject") != -1)
					{
						m_LuaStackObjectPtr = address;
						m_evaluateSymbolRadio.SetCheck(1);
					}
				}
			}
		}
	}

	if (m_processHandle)
		CloseHandle(m_processHandle);
	m_processHandle = OpenProcess(PROCESS_VM_READ, FALSE, m_processID);
	g_processHandle = m_processHandle;

	if (m_LuaStackObjectPtr)
	{
		LuaStackObject luaStackObject;
		ReadMem_LuaStackObject( NULL, m_LuaStackObjectPtr, luaStackObject );
		m_LuaStatePtr = luaStackObject.m_state;
	}

	if (m_LuaObjectPtr)
	{
		LuaObject luaObject;
		ReadMem_LuaObject( NULL, m_LuaObjectPtr, luaObject );
		m_LuaStatePtr = luaObject.m_state;
	}

	if (m_LuaStateOwnerPtr)
	{
		ReadMem( NULL, m_LuaStateOwnerPtr, &m_LuaStatePtr, sizeof( void* ) );
		SwapEndian(m_LuaStatePtr);
	}

	if (m_LuaStatePtr)
	{
		LuaState luaState;
		ReadMem( NULL, m_LuaStatePtr, &luaState, sizeof( LuaState ) );
		SwapEndian(luaState.m_state);
		m_luaStatePtr = luaState.m_state;
	}

	ReadMem(NULL, m_luaStatePtr, &theState, sizeof(lua_State));
	BYTE* from = (BYTE*)m_luaStatePtr;
	ReadMem(NULL, from + _offsetof(lua_State, l_G), &theState.l_G, sizeof(theState.l_G));
	SwapEndian(theState.l_G);
	ReadMem_TValue(NULL, from + _offsetof(lua_State, l_gt), theState.l_gt);
}


LRESULT CWatchCtl::OnTvnItemexpandingTables(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	CTreeItem treeItem(pNMTreeView->itemNew.hItem, &m_tableTree);
	OnPlusExpand(treeItem);

	return 0;
}

LRESULT CWatchCtl::OnTvnSelchangedTables(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	CTreeItem treeItem(pNMTreeView->itemNew.hItem, &m_tableTree);
	TValue table = *(TValue*)treeItem.GetData();
	FillInKeyValues(&table, NULL);

	return 0;
}

LRESULT CWatchCtl::OnBnClickedGlobals(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	Refresh();

	return 0;
}

LRESULT CWatchCtl::OnBnClickedRegistry(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	Refresh();

	return 0;
}


LRESULT CWatchCtl::OnBnClickedLocals(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	Refresh();

	return 0;
}


LRESULT CWatchCtl::OnBnClickedEvaluatesymbol(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	Refresh();

	return 0;
}

LRESULT CWatchCtl::OnEnChangeSymbol(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_evaluateSymbolRadio.SetCheck(1);

	return 0;
}

LRESULT CWatchCtl::OnBnClickedAccept(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	Refresh();

	return 0;
}
