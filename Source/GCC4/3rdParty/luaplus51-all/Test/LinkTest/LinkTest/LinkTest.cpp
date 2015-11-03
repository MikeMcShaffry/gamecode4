// LinkTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "LuaPlus.h"

static const char* METATABLE_NAME = "BaseScriptComponentMetaTable";

static LuaPlus::LuaState *g_pLuaState = NULL;


class BaseScriptComponent 
{
//	std::string m_scriptObjectName;
//	std::string m_constructorName;
//	std::string m_destructorName;

	LuaPlus::LuaObject m_scriptObject;
	LuaPlus::LuaObject m_scriptConstructor;
	LuaPlus::LuaObject m_scriptDestructor;

public:
	static const char* g_Name;
	virtual const char* VGetName() const { return g_Name; }

	BaseScriptComponent(void) { };
	virtual ~BaseScriptComponent(void) { };

	static void RegisterScriptFunctions(void) { };
	static void UnregisterScriptFunctions(void) { };

	void CreateScriptObject(void) { };

	// component script functions
	LuaPlus::LuaObject GetActorId(void);

	// physics component script functions
	LuaPlus::LuaObject GetPos(void);
	void SetPos(LuaPlus::LuaObject newPos) { };
	LuaPlus::LuaObject GetLookAt(void) const;
};


LuaPlus::LuaObject BaseScriptComponent::GetLookAt(void) const
{
	LuaPlus::LuaObject ret;

	ret.AssignNil(g_pLuaState);

	return ret;
}

LuaPlus::LuaObject BaseScriptComponent::GetActorId(void)
{
	//GCC_LOG("ObjectSystem", "BaseScriptComponent::GetEntityId() return 0x" + ToStr(m_pOwner->GetId(), 16) + " on C++ side");

	// [rez] Wtf??  Returning an unsigned int here will produce really odd results in Release mode (debug is 
	// fine).  The number will sometimes get truncated, so 0xb1437643 might become 0xb1437600.  Even though the 
	// C++ side is getting the right number, it gets mangled on the way out to Lua.  The somewhat hacky fix here 
	// is to return a LuaPlus::LuaObject, which seems to work just fine.  I'm not sure if this will cause any 
	// efficiency issues, but I'm guessing the LuaPlus internals do something like this anyway to marshal the 
	// data back to Lua land, so it's probably okay.
	LuaPlus::LuaObject ret;
	ret.AssignInteger(g_pLuaState, 0);
	return ret;

	//return m_pOwner->GetId();
}

LuaPlus::LuaObject BaseScriptComponent::GetPos(void)
{
	LuaPlus::LuaObject ret;
	ret.AssignNil(g_pLuaState);
	return ret;
}



int _tmain(int argc, _TCHAR* argv[])
{
	g_pLuaState = LuaPlus::LuaState::Create(true);

	// create the metatable
	LuaPlus::LuaObject metaTableObj = g_pLuaState->GetGlobals().CreateTable(METATABLE_NAME);
	metaTableObj.SetObject("__index", metaTableObj);

	// transform component functions
	metaTableObj.RegisterObjectDirect("GetActorId", (BaseScriptComponent*)0, &BaseScriptComponent::GetActorId);
	metaTableObj.RegisterObjectDirect("GetPos", (BaseScriptComponent*)0, &BaseScriptComponent::GetPos);
	metaTableObj.RegisterObjectDirect("SetPos", (BaseScriptComponent*)0, &BaseScriptComponent::SetPos);

	return 0;
}

