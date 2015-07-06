#include "RemoteLuaDebugger_InternalPch.h"
#include "RemoteLuaDebugger.h"

#include "RemoteLuaDebuggerDoc.h"

using namespace ATL;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRemoteLuaDebuggerDoc

IMPLEMENT_DYNCREATE(CRemoteLuaDebuggerDoc, CDocument)

BEGIN_MESSAGE_MAP(CRemoteLuaDebuggerDoc, CDocument)
	//{{AFX_MSG_MAP(CRemoteLuaDebuggerDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRemoteLuaDebuggerDoc construction/destruction

CMap<CString, LPCTSTR, int, int> m_receivedFileMap;

static int LS_DebugUpdateWatch( LuaState* state )
{
	theApp.GetDebuggerView()->LS_DebugUpdateWatch( state );
	return 0;
}


static int Script_DebugUpdateCallStack( LuaState* state )
{
	LuaStack args(state);
	CString stateName = args[1].GetString();
	theApp.GetDebuggerView()->DebugUpdateCallStack(stateName);
	return 0;
}

static int Script_DebugOutput( LuaState* state )
{
	LuaStack args(state);
	CString stateName = args[1].GetString();
	LuaStackObject outputObj( state, 2 );
	CString outputStr = outputObj.GetString();
	theApp.GetMainFrame()->GetOutputBar().AppendLuaCode( outputStr );
	return 0;
}


static int Script_DebugSendFile( LuaState* state )
{
	LuaStackObject filenameObj(state, 1);
	CString fileName = filenameObj.GetString();

	CString fileNameNoPath = fileName;
	int slashPos = fileNameNoPath.ReverseFind('/');
	fileNameNoPath = fileNameNoPath.Mid(slashPos + 1);

	BYTE* buffer;
	int size;
	if (theApp.GetNetworkClient().GetSocket().ReadBinary(buffer, size))
	{
		CFile file;
		if (!file.Open(theApp.GetCachePath() + fileNameNoPath, CFile::modeCreate | CFile::modeWrite))
		{

		}

		file.Write(buffer, size);

		file.Close();

		delete [] buffer;

		m_receivedFileMap[fileNameNoPath] = 1;

		theApp.GetMainFrame()->GetWindowListBar().Refresh();
	}

	return 0;
}


static int Script_DebugSetLocation( LuaState* state )
{
	LuaStackObject stateObj( state, 1 );
	LuaStackObject filenameObj( state, 2 );
	LuaStackObject lineNumberObj( state, 3 );

	const char* stateStr = stateObj.GetString();

	CString fileName = filenameObj.GetString();
	fileName.MakeLower();
	int lineNumber = lineNumberObj.GetInteger();

	CString fileNameNoPath = fileName;
	int slashPos = fileNameNoPath.ReverseFind('/');
	fileNameNoPath = fileNameNoPath.Mid(slashPos + 1);

	int temp;
	if (!m_receivedFileMap.Lookup(fileNameNoPath, temp))
	{
		// Go to the next line.
		CString command;
		command.Format(_T("DebugStepOut()"));
		theApp.GetNetworkClient().SendCommand(command);
	}
	else
	{
		theApp.GetDebuggerView()->DebugSetLocation(stateStr, fileNameNoPath, lineNumber);
	}

	return 0;
}


static int Script_DebugUpdateLocals( LuaState* state )
{
	theApp.GetDebuggerView()->LS_DebugUpdateLocals( state );
	return 0;
}


static int Script_LuaServerConnect( LuaState* state )
{
	m_receivedFileMap.RemoveAll();
	theApp.GetDebuggerView()->m_currentFileName.Empty();
	theApp.GetNetworkClient().SendCommand("LuaClientConnect()");
	return 0;
}


static int Script_LuaDebuggerConnect(LuaState* state)
{
	LuaStack args(state);
	CString stateName = args[1].GetString();
	theApp.GetNetworkClient().SendCommand("DebugOn(\"" + stateName + "\")");
	theApp.GetDebuggerView()->DebugAddAllWatches(stateName);
	theApp.GetDebuggerView()->DebugSendAllBreakpoints(stateName);
	return 0;
}


void LuaDebugClient::OnConnect()
{
	theApp.GetNetworkClient().SendCommand("LuaClientConnect()");
}

	
CRemoteLuaDebuggerDoc::CRemoteLuaDebuggerDoc()
{
	m_state = LuaState::Create();
	LuaObject globalsObj = m_state->GetGlobals();
	globalsObj.Register("DebugSendFile", Script_DebugSendFile);
	globalsObj.Register("DebugSetLocation", Script_DebugSetLocation);
	globalsObj.Register("DebugUpdateLocals", Script_DebugUpdateLocals);
	globalsObj.Register("DebugUpdateWatch", LS_DebugUpdateWatch );
	globalsObj.Register("DebugUpdateCallStack", Script_DebugUpdateCallStack);
	globalsObj.Register("DebugOutput", Script_DebugOutput);
	globalsObj.Register("LuaServerConnect", Script_LuaServerConnect);
	globalsObj.Register("LuaDebuggerConnect", Script_LuaDebuggerConnect);

	const DWORD LUA_DEBUGGER_PORT = 3536;
	theApp.GetNetworkClient().Open("LuaDebugger", CStringA(theApp.m_pszRegistryKey), m_state->GetCState(), NULL, LUA_DEBUGGER_PORT);
}


CRemoteLuaDebuggerDoc::~CRemoteLuaDebuggerDoc()
{
	LuaState::Destroy( m_state );
}

BOOL CRemoteLuaDebuggerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CRemoteLuaDebuggerDoc serialization

void CRemoteLuaDebuggerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CRemoteLuaDebuggerDoc diagnostics

#ifdef _DEBUG
void CRemoteLuaDebuggerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CRemoteLuaDebuggerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CRemoteLuaDebuggerDoc commands

CDebuggerView* CRemoteLuaDebuggerDoc::GetDebuggerView()
{
	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		if (pView->IsKindOf(RUNTIME_CLASS(CDebuggerView)))
		{
			return (CDebuggerView*)pView;
		}
	}

	ASSERT(0);
	return NULL;
}


void CRemoteLuaDebuggerDoc::OnIdleCustom()
{
	theApp.GetNetworkClient().ProcessPackets();
}


void CRemoteLuaDebuggerDoc::GetActiveFiles(CStringArray& fileList)
{
	fileList.RemoveAll();

	POSITION pos = m_receivedFileMap.GetStartPosition();
	while (pos)
	{
		CString fileName;
		int temp;
		m_receivedFileMap.GetNextAssoc(pos, fileName, temp);

		fileList.Add(fileName);
	}
}
