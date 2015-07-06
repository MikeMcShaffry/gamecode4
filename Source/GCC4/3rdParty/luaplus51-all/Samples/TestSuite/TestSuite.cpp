// TestSuite.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../TestScript/SimpleHeap.h"
#include <assert.h>
#include "UnitTest++.h"
#include <list>

//////////////////////////////////////////////////////////////////////////
TEST(LuaState_creation1)
{
	LuaState* state = LuaState::Create(false);
	CHECK(state);
	LuaState::Destroy(state);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_creation2)
{
	LuaState* state = LuaState::Create(true);
	CHECK(state);
	LuaState::Destroy(state);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaStateOwner_creationDestruction_1)
{
	LuaStateOwner state(false);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaStateOwner_creationDestruction_2)
{
	LuaStateOwner state(true);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_CreateThread)
{
	LuaStateOwner state(false);
	LuaObject threadObj = LuaState::CreateThread(state);
	CHECK(threadObj.Type() == LUA_TTHREAD);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_CastState)
{
	LuaStateOwner state(false);
	lua_State* L = state->GetCState();
	LuaState* state2 = lua_State_To_LuaState(L);
	CHECK(state == state2);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_Stack)
{
	LuaStateOwner state(false);
	state->PushNumber(5);
	state->PushNumber(10);

	CHECK(state->Stack(-1).IsNumber());
	CHECK(state->Stack(-1).GetNumber() == 10);
	CHECK(state->Stack(-2).IsNumber());
	CHECK(state->Stack(-2).GetNumber() == 5);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_StackTop)
{
	LuaStateOwner state(false);
	state->PushNumber(5);
	state->PushNumber(10);

	CHECK(state->StackTop().IsNumber());
	CHECK(state->StackTop().GetNumber() == 10);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_GetTop)
{
	LuaStateOwner state(false);
	CHECK_EQUAL(state->GetTop(), 0);

	state->PushNumber(5);
	state->PushNumber(10);

	CHECK_EQUAL(state->GetTop(), 2);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_SetTop)
{
	LuaStateOwner state(false);
	CHECK_EQUAL(state->GetTop(), 0);

	state->PushNumber(5);
	state->PushNumber(10);
	state->PushNumber(15);

	CHECK(state->Stack(-1).IsNumber());
	CHECK_EQUAL(15, (int)state->Stack(-1).GetNumber());
	CHECK(state->Stack(-2).IsNumber());
	CHECK_EQUAL(10, (int)state->Stack(-2).GetNumber());
	CHECK(state->Stack(-3).IsNumber());
	CHECK_EQUAL(5, (int)state->Stack(-3).GetNumber());

	state->SetTop(1);
	CHECK(state->Stack(-1).IsNumber());
	CHECK_EQUAL(5, (int)state->Stack(-1).GetNumber());
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_PushValue)
{
	LuaStateOwner state(false);
	state->PushValue(LUA_GLOBALSINDEX);
	state->PushNumber(5);
	state->PushNumber(10);
	state->PushNumber(15);
	state->PushValue(-2);

	CHECK_EQUAL(state->Stack(-1).GetNumber(), 10);
	CHECK_EQUAL(state->Stack(-2).GetNumber(), 15);
	CHECK_EQUAL(state->Stack(-3).GetNumber(), 10);
	CHECK_EQUAL(state->Stack(-4).GetNumber(), 5);
	CHECK(state->Stack(-5).IsTable());
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_PushValueStackObject)
{
	LuaStateOwner state(false);
	state->PushNumber(5);
	state->PushValue(state->Stack(-1));

	CHECK_EQUAL(state->Stack(-1).GetNumber(), 5);
	CHECK_EQUAL(state->Stack(-2).GetNumber(), 5);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_Remove)
{
	LuaStateOwner state(false);
	CHECK_EQUAL(state->GetTop(), 0);

	state->PushNumber(5);
	state->PushNumber(10);
	state->PushNumber(15);

	state->Remove(-2);
	CHECK(state->Stack(-1).IsNumber());
	CHECK_EQUAL(15, (int)state->Stack(-1).GetNumber());
	CHECK(state->Stack(-2).IsNumber());
	CHECK_EQUAL(5, (int)state->Stack(-2).GetNumber());
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_Insert)
{
	LuaStateOwner state(false);
	CHECK_EQUAL(state->GetTop(), 0);

	state->PushNumber(5);
	state->PushNumber(10);
	state->PushNumber(15);

	state->Insert(-2);
	CHECK_EQUAL(state->GetTop(), 3);
	CHECK(state->Stack(-1).IsNumber());
	CHECK_EQUAL(10, (int)state->Stack(-1).GetNumber());
	CHECK(state->Stack(-2).IsNumber());
	CHECK_EQUAL(15, (int)state->Stack(-2).GetNumber());
	CHECK(state->Stack(-3).IsNumber());
	CHECK_EQUAL(5, (int)state->Stack(-3).GetNumber());
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_Replace)
{
	LuaStateOwner state(false);
	CHECK_EQUAL(state->GetTop(), 0);

	state->PushNumber(5);
	state->PushNumber(10);
	state->PushNumber(15);

	state->Replace(-2);
	CHECK_EQUAL(state->GetTop(), 2);
	CHECK(state->Stack(-1).IsNumber());
	CHECK_EQUAL(15, (int)state->Stack(-1).GetNumber());
	CHECK(state->Stack(-2).IsNumber());
	CHECK_EQUAL(5, (int)state->Stack(-2).GetNumber());
}


/**TODO: LuaState::CheckStack**/
/**TODO: LuaState::XMove**/


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_Equal)
{
	LuaStateOwner state(false);
	LuaObject stringObj;
	stringObj.AssignString(state, "Hello");
	state->DoString("MyString = 'Hello'; MyString2 = 'Hi'");
	LuaObject string2Obj = state->GetGlobal("MyString");
	CHECK(state->Equal(stringObj, string2Obj));
	string2Obj = state->GetGlobal("MyString2");
	CHECK(!state->Equal(stringObj, string2Obj));
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_Equal_2)
{
	LuaStateOwner state(false);
	state->PushNumber(5);
	state->PushNumber(10);
	state->PushNumber(5);

	CHECK_EQUAL(state->Equal(-1, -3), 1);
	CHECK_EQUAL(state->Equal(-1, -2), 0);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_RawEqual)
{
	LuaStateOwner state(false);
	state->PushNumber(5);
	state->PushNumber(10);
	state->PushNumber(5);

	CHECK_EQUAL(state->RawEqual(-1, -3), 1);
	CHECK_EQUAL(state->RawEqual(-1, -2), 0);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_LessThan)
{
	LuaStateOwner state(false);
	LuaObject stringObj;
	stringObj.AssignString(state, "Hello");
	state->DoString("MyString = 'Hello'; MyString2 = 'Hi'");
	LuaObject string2Obj = state->GetGlobal("MyString");
	CHECK(!state->LessThan(stringObj, string2Obj));
	string2Obj = state->GetGlobal("MyString2");
	CHECK(state->LessThan(stringObj, string2Obj));
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_LessThan_2)
{
	LuaStateOwner state(false);
	state->PushNumber(5);
	state->PushNumber(10);
	state->PushNumber(5);

	CHECK_EQUAL(state->LessThan(-3, -2), 1);
	CHECK_EQUAL(state->LessThan(-2, -3), 0);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_PushNil)
{
	LuaStateOwner state(false);
	LuaStackObject obj = state->PushNil();
	CHECK_EQUAL(state->GetTop(), 1);
	CHECK(obj.IsNil());
	CHECK(state->Stack(-1).IsNil());
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_PushNumber)
{
	LuaStateOwner state(false);
	LuaStackObject obj = state->PushNumber(5);
	CHECK_EQUAL(state->GetTop(), 1);
	CHECK(obj.IsNumber());
	CHECK(obj.IsInteger());
	CHECK_EQUAL(obj.GetNumber(), 5);
	CHECK(state->Stack(-1).IsNumber());
	CHECK(state->Stack(-1).IsInteger());
	CHECK_EQUAL(state->Stack(-1).GetNumber(), 5);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_PushInteger)
{
	LuaStateOwner state(false);
	LuaStackObject obj = state->PushInteger(5);
	CHECK_EQUAL(state->GetTop(), 1);
	CHECK(obj.IsNumber());
	CHECK(obj.IsInteger());
	CHECK_EQUAL(obj.GetNumber(), 5);
	CHECK(state->Stack(-1).IsNumber());
	CHECK(state->Stack(-1).IsInteger());
	CHECK_EQUAL(state->Stack(-1).GetNumber(), 5);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_PushLString)
{
	LuaStateOwner state(false);
	LuaStackObject obj = state->PushLString("Hello, world", 5);
	CHECK_EQUAL(state->GetTop(), 1);
	CHECK(obj.IsString());
	CHECK_EQUAL(obj.GetString(), "Hello");
	CHECK(state->Stack(-1).IsString());
	CHECK_EQUAL(state->Stack(-1).GetString(), "Hello");
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_PushLWString)
{
	LuaStateOwner state(false);
	lua_WChar str[] = { 'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', 0 };
	lua_WChar compareStr[] = { 'H', 'e', 'l', 'l', 'o', 0 };
	LuaStackObject obj = state->PushLWString(str, 5);
	CHECK_EQUAL(state->GetTop(), 1);
	CHECK(obj.IsWString());
	CHECK_EQUAL(lp_wcscmp(obj.GetWString(), compareStr), 0);
	CHECK(state->Stack(-1).IsWString());
	CHECK_EQUAL(lp_wcscmp(state->Stack(-1).GetWString(), compareStr), 0);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_PushString)
{
	LuaStateOwner state(false);
	LuaStackObject obj = state->PushString("Hello");
	CHECK_EQUAL(state->GetTop(), 1);
	CHECK(obj.IsString());
	CHECK_EQUAL(obj.GetString(), "Hello");
	CHECK(state->Stack(-1).IsString());
	CHECK_EQUAL(state->Stack(-1).GetString(), "Hello");
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_PushWString)
{
	LuaStateOwner state(false);
	lua_WChar str[] = { 'H', 'e', 'l', 'l', 'o', 0 };
	lua_WChar compareStr[] = { 'H', 'e', 'l', 'l', 'o', 0 };
	LuaStackObject obj = state->PushWString(str);
	CHECK_EQUAL(state->GetTop(), 1);
	CHECK(obj.IsWString());
	CHECK_EQUAL(lp_wcscmp(obj.GetWString(), compareStr), 0);
	CHECK(state->Stack(-1).IsWString());
	CHECK_EQUAL(lp_wcscmp(state->Stack(-1).GetWString(), compareStr), 0);
}


//////////////////////////////////////////////////////////////////////////
LuaStackObject LuaState_PushVFStringHelper(LuaState* state, const char* fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	LuaStackObject obj = state->PushVFString(fmt, argp);
	va_end(argp);
	return obj;
}


TEST(LuaState_PushVFString)
{
	LuaStateOwner state(false);
	LuaStackObject obj = LuaState_PushVFStringHelper(state, "%s%d", "Hello", 5);
	CHECK_EQUAL(state->GetTop(), 1);
	CHECK(obj.IsString());
	CHECK_EQUAL(obj.GetString(), "Hello5");
	CHECK(state->Stack(-1).IsString());
	CHECK_EQUAL(state->Stack(-1).GetString(), "Hello5");
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_PushFString)
{
	LuaStateOwner state(false);
	LuaStackObject obj = state->PushFString("%s%d", "Hello", 5);
	CHECK_EQUAL(state->GetTop(), 1);
	CHECK(obj.IsString());
	CHECK_EQUAL(obj.GetString(), "Hello5");
	CHECK(state->Stack(-1).IsString());
	CHECK_EQUAL(state->Stack(-1).GetString(), "Hello5");
}


//////////////////////////////////////////////////////////////////////////
int var_LuaState_PushCClosure_Helper_1 = 0;
static int LuaState_PushCClosure_Helper_1(lua_State* L)
{
	var_LuaState_PushCClosure_Helper_1 = 1;
	return 0;
}

TEST(LuaState_PushCClosure_1)
{
	LuaStateOwner state(false);
	LuaStackObject obj = state->PushCClosure(LuaState_PushCClosure_Helper_1, 0);
	CHECK_EQUAL(state->GetTop(), 1);
	CHECK(obj.IsCFunction());
	CHECK(obj.GetCFunction() == LuaState_PushCClosure_Helper_1);
	CHECK(state->Stack(-1).IsCFunction());
	CHECK(state->Stack(-1).GetCFunction() == LuaState_PushCClosure_Helper_1);

	CHECK_EQUAL(var_LuaState_PushCClosure_Helper_1, 0);
	state->PCall(0, 0, 0);
	CHECK_EQUAL(var_LuaState_PushCClosure_Helper_1, 1);
}


//////////////////////////////////////////////////////////////////////////
int var_LuaState_PushCClosure_Helper_2 = 0;
static int LuaState_PushCClosure_Helper_2(LuaState* state)
{
	var_LuaState_PushCClosure_Helper_2 = 1;
	return 0;
}

TEST(LuaState_PushCClosure_2)
{
	LuaStateOwner state(false);
	LuaStackObject obj = state->PushCClosure(LuaState_PushCClosure_Helper_2, 0);
	CHECK_EQUAL(state->GetTop(), 1);
	CHECK(obj.IsCFunction());
	CHECK(state->Stack(-1).IsCFunction());

	CHECK_EQUAL(var_LuaState_PushCClosure_Helper_2, 0);
	state->PCall(0, 0, 0);
	CHECK_EQUAL(var_LuaState_PushCClosure_Helper_2, 1);
}


//////////////////////////////////////////////////////////////////////////
int var_LuaState_PushCClosure_Helper_3 = 0;
static int LuaState_PushCClosure_Helper_3(lua_State* L)
{
	var_LuaState_PushCClosure_Helper_3 = 1;
	return 0;
}

TEST(LuaState_PushCClosure_3)
{
	LuaStateOwner state(false);
	LuaStackObject obj = state->PushCClosure(LuaState_PushCClosure_Helper_3, 0);
	CHECK_EQUAL(state->GetTop(), 1);
	CHECK(obj.IsCFunction());
	CHECK(obj.GetCFunction() == LuaState_PushCClosure_Helper_3);
	CHECK(state->Stack(-1).IsCFunction());
	CHECK(state->Stack(-1).GetCFunction() == LuaState_PushCClosure_Helper_3);

	CHECK_EQUAL(var_LuaState_PushCClosure_Helper_3, 0);
	state->PCall(0, 0, 0);
	CHECK_EQUAL(var_LuaState_PushCClosure_Helper_3, 1);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_PushBoolean)
{
	LuaStateOwner state(false);
	LuaStackObject obj = state->PushBoolean(true);
	CHECK_EQUAL(state->GetTop(), 1);
	CHECK(obj.IsBoolean());
	CHECK_EQUAL(obj.GetBoolean(), true);
	CHECK(state->Stack(-1).IsBoolean());
	CHECK_EQUAL(state->Stack(-1).GetBoolean(), true);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_PushLightUserData)
{
	LuaStateOwner state(false);
	LuaStackObject obj = state->PushLightUserData((void*)0x12345678);
	CHECK_EQUAL(state->GetTop(), 1);
	CHECK(obj.IsLightUserData());
	CHECK(obj.GetLightUserData() == (void*)0x12345678);
	CHECK(state->Stack(-1).IsLightUserData());
	CHECK(state->Stack(-1).GetLightUserData() == (void*)0x12345678);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_PushThread)
{
	LuaStateOwner state(false);
	LuaStackObject obj = state->PushThread();
	CHECK_EQUAL(state->GetTop(), 1);
	CHECK(obj.IsThread());
	CHECK(obj.GetThread() == *state);
	CHECK(state->Stack(-1).IsThread());
	CHECK(state->Stack(-1).GetThread() == *state);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_CreateTable)
{
	LuaStateOwner state(false);
	LuaStackObject obj = state->CreateTable();
	CHECK_EQUAL(state->GetTop(), 1);
	CHECK(obj.IsTable());
	CHECK(state->Stack(-1).IsTable());
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_NewUserData)
{
	LuaStateOwner state(false);
	LuaStackObject obj = state->NewUserData(10);
	CHECK_EQUAL(state->GetTop(), 1);
	CHECK(obj.IsUserData());
	CHECK(!obj.IsLightUserData());
	CHECK(state->Stack(-1).IsUserData());
	CHECK(!state->Stack(-1).IsLightUserData());
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_GetGlobals)
{
	LuaStateOwner state(false);
	lua_State* L = state->GetCState();
	lua_pushvalue(L, LUA_GLOBALSINDEX);
	state->GetGlobals().Push();
	CHECK(lua_equal(L, -1, -2));
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_GetGlobals_Stack)
{
	LuaStateOwner state(false);
	lua_State* L = state->GetCState();
	lua_pushvalue(L, LUA_GLOBALSINDEX);
	state->GetGlobals_Stack().Push();
	CHECK(lua_equal(L, -1, -2));
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_GetGlobal)
{
	LuaStateOwner state(false);
	state->DoString("MyGlobal = 5");
	CHECK(state->GetGlobal("MyGlobal").GetInteger() == 5);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_GetRegistry)
{
	LuaStateOwner state(false);
	lua_State* L = state->GetCState();
	lua_pushvalue(L, LUA_REGISTRYINDEX);
	state->GetRegistry().Push();
	CHECK(lua_equal(L, -1, -2));
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_GetRegistry_Stack)
{
	LuaStateOwner state(false);
	lua_State* L = state->GetCState();
	lua_pushvalue(L, LUA_REGISTRYINDEX);
	state->GetRegistry_Stack().Push();
	CHECK(lua_equal(L, -1, -2));
}


//////////////////////////////////////////////////////////////////////////
static int LuaState_Call_Helper(LuaState* state)
{
	LuaStack args(state);
	state->PushNumber(args[1].GetNumber() + args[2].GetNumber());
	return 1;
}

TEST(LuaState_Call)
{
	LuaStateOwner state(false);
	state->PushCClosure(LuaState_Call_Helper, 0);

	state->PushNumber(5);
	state->PushNumber(4);
	state->Call(2, 1);
	CHECK_EQUAL(state->Stack(-1).GetNumber(), 9);
}


//////////////////////////////////////////////////////////////////////////
static int LuaState_PCall_Helper(LuaState* state)
{
	LuaStack args(state);
	state->PushNumber(args[1].GetNumber() + args[2].GetNumber());
	return 1;
}

TEST(LuaState_PCall)
{
	LuaStateOwner state(false);
	state->PushCClosure(LuaState_PCall_Helper, 0);

	state->PushNumber(5);
	state->PushNumber(4);
	state->PCall(2, 1, 0);
	CHECK_EQUAL(state->Stack(-1).GetNumber(), 9);
}


//////////////////////////////////////////////////////////////////////////
static int LuaState_CPCall_Helper(lua_State* L)
{
	LuaState* state = lua_State_To_LuaState(L);
	int* var = (int*)state->Stack(1).GetLightUserData();
	*var = 5;
	return 0;
}

TEST(LuaState_CPCall)
{
	int var = 0;
	LuaStateOwner state(false);
	state->CPCall(LuaState_CPCall_Helper, &var);
	CHECK_EQUAL(var, 5);
}


//////////////////////////////////////////////////////////////////////////
struct LuaState_Load_Info
{
	LuaState_Load_Info() : pos(0), size(0)  {}

	char buffer[1000];
	size_t pos;
	size_t size;
};


static const char* LuaState_Load_Helper_Get(lua_State *L, void *ud, size_t *size)
{
	LuaState_Load_Info* loadInfo = (LuaState_Load_Info*)ud;
	(void)L;
	if (loadInfo->pos == loadInfo->size)
		return NULL;
	*size = 1;
	return loadInfo->buffer + loadInfo->pos++;
}


TEST(LuaState_Load)
{
	LuaStateOwner state(false);

	LuaState_Load_Info loadInfo;
	strcpy(loadInfo.buffer, "MyNumber = 5");
	loadInfo.size = strlen(loadInfo.buffer);

	int ret = state->Load(LuaState_Load_Helper_Get, &loadInfo, NULL);
	CHECK_EQUAL(0, ret);

	state->PCall(0, 0, 0);
	LuaObject obj = state->GetGlobals()["MyNumber"];
	CHECK(obj.IsNumber());
	CHECK_EQUAL(5, obj.GetNumber());
}


//////////////////////////////////////////////////////////////////////////
struct LuaState_WLoad_Info
{
	LuaState_WLoad_Info() : pos(0), size(0)  {}

	lua_WChar buffer[1000];
	size_t pos;
	size_t size;
};


static const char* LuaState_WLoad_Helper_Get(lua_State *L, void *ud, size_t *size)
{
	LuaState_WLoad_Info* loadInfo = (LuaState_WLoad_Info*)ud;
	(void)L;
	if (loadInfo->pos == loadInfo->size)
		return NULL;
	*size = 2;
	return (const char*)(loadInfo->buffer + loadInfo->pos++);
}


TEST(LuaState_WLoad)
{
	LuaStateOwner state(false);

	LuaState_WLoad_Info loadInfo;
	lua_WChar str[] = { 'M', 'y', 'N', 'u', 'm', 'b', 'e', 'r', ' ' , '=', ' ', '5', 0 };
	memcpy(loadInfo.buffer, str, sizeof(str));
	loadInfo.size = lp_wcslen(str);

	int ret = state->WLoad(LuaState_WLoad_Helper_Get, &loadInfo, NULL);
	CHECK_EQUAL(0, ret);

	state->PCall(0, 0, 0);
	LuaObject obj = state->GetGlobals()["MyNumber"];
	CHECK(obj.IsNumber());
	CHECK_EQUAL(5, obj.GetNumber());
}


//////////////////////////////////////////////////////////////////////////
struct LuaState_Dump_Info
{
	LuaState_Dump_Info() : bufferPos(0)  {}

	char buffer[1000];
	int bufferPos;
};


static int LuaState_Dump_Helper(lua_State* L, const void* p, size_t sz, void* ud)
{
	LuaState_Dump_Info* dumpInfo = (LuaState_Dump_Info*)ud;
	memcpy(&dumpInfo->buffer[dumpInfo->bufferPos], p, sz);
	dumpInfo->bufferPos += sz;
	return 0;
}


TEST(LuaState_Dump)
{
	LuaStateOwner state(false);
	int ret = state->LoadString("MyNumber = 5");
	CHECK_EQUAL(state->GetTop(), 1);

	LuaState_Dump_Info dumpInfo;	
	state->Dump(LuaState_Dump_Helper, &dumpInfo, 1, '=');
	state->Pop();
	CHECK(dumpInfo.bufferPos > 0);

	ret = state->LoadBuffer(dumpInfo.buffer, dumpInfo.bufferPos, "Compiled Buffer");
	CHECK_EQUAL(0, ret);

	state->PCall(0, 0, 0);
	LuaObject obj = state->GetGlobals()["MyNumber"];
	CHECK(obj.IsNumber());
	CHECK_EQUAL(5, obj.GetNumber());
}


/**TODO:LuaState.CoYield**/
/**TODO:LuaState.CoResume**/
/**TODO:LuaState.CoStatus**/
/**TODO:LuaState.GC**/


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_Error)
{
	LuaStateOwner state(false);
	try
	{
//		state->Error();
	}
	catch (LuaException&)
	{
	}
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_Next)
{
	LuaStateOwner state(false);
	state->DoString("MyTable = { 5, 4, 3, 2, 1 }");

	int count = 0;
	state->GetGlobal_Stack("MyTable");
	state->PushNil();
	while (state->Next(-2) != 0)
	{
		CHECK(state->Stack(-2).IsNumber());
		CHECK(state->Stack(-2).GetNumber() == 1 + count);
		CHECK(state->Stack(-1).IsNumber());
		CHECK(state->Stack(-1).GetNumber() == 5 - count);
		state->Pop();
		++count;
	}
	CHECK_EQUAL(5, count);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_Concat)
{
	LuaStateOwner state(false);
	state->PushString("Ab");
	state->PushString("Cd");
	state->PushString("Ef");
	state->PushString("Gh");
	state->PushString("Ij");
	state->Concat(5);
	CHECK_EQUAL(1, state->GetTop());
	CHECK(state->Stack(-1).IsString());
	CHECK_EQUAL("AbCdEfGhIj", state->Stack(-1).GetString());
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaState_ConcatW)
{
	lua_WChar str1[] = { 'A', 'b', 0 };
	lua_WChar str2[] = { 'C', 'd', 0 };
	lua_WChar str3[] = { 'E', 'f', 0 };
	lua_WChar str4[] = { 'G', 'h', 0 };
	lua_WChar str5[] = { 'I', 'j', 0 };
	LuaStateOwner state(false);
	state->PushWString(str1);
	state->PushWString(str2);
	state->PushWString(str3);
	state->PushWString(str4);
	state->PushWString(str5);
	state->Concat(5);
	CHECK_EQUAL(1, state->GetTop());
	CHECK(state->Stack(-1).IsWString());
	lua_WChar finalStr[] = { 'A', 'b', 'C', 'd', 'E', 'f', 'G', 'h', 'I', 'j', 0 };
	CHECK_EQUAL(0, lp_wcscmp(finalStr, state->Stack(-1).GetWString()));
}


/**TODO: LuaState.GetAllocF**/
/**TODO: LuaState.SetAllocF**/

//////////////////////////////////////////////////////////////////////////
TEST(LuaState_Pop)
{
	LuaStateOwner state(false);
	state->PushNumber(1);
	state->PushNumber(2);
	state->PushNumber(3);
	state->PushNumber(4);
	CHECK_EQUAL(4, state->GetTop());
	state->Pop();
	CHECK_EQUAL(3, state->GetTop());
	state->Pop(2);
	CHECK_EQUAL(1, state->GetTop());
}


//////////////////////////////////////////////////////////////////////////
/*int CountLuaObjects(LuaState* state)
{
	int count = 0;
	LuaObject* curObj = state->GetHeadObject()->m_next;
	while (curObj != state->GetTailObject())
	{
		count++;
		curObj = curObj->m_next;		
	}

	return count;
}
*/

//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_CreationBareConstructor)
{
	LuaObject obj;
	CHECK(obj.GetState() == NULL);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_CreationWithCounting)
{
	{
		LuaStateOwner state;
		LuaObject obj(state);
//		CHECK(CountLuaObjects(state) == 1);
	}

	{
		LuaStateOwner state;
		LuaObject obj(state);
		{
			LuaObject obj2(state);
//			CHECK(CountLuaObjects(state) == 2);
		}
//		CHECK(CountLuaObjects(state) == 1);
	}

	{
		LuaStateOwner state;
		LuaObject obj(state);
		{
			LuaObject obj2(state);
			LuaObject obj3(state);
//			CHECK(CountLuaObjects(state) == 3);
		}
//		CHECK(CountLuaObjects(state) == 1);
	}
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_CreationBareConstructorWithAssignment)
{
	LuaStateOwner state;
	LuaObject obj;
	CHECK(obj.GetState() == NULL);
	obj = state->GetGlobals();
	CHECK(obj.GetState() == state);
	CHECK(obj.Type() == LUA_TTABLE);
	CHECK(obj.IsTable());
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_CreationWithLuaStatePointer)
{
	LuaStateOwner state;
	LuaObject obj(state);
	CHECK(obj.IsNil());
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_CreationWithLuaStatePointerAndStackIndex)
{
	LuaStateOwner state;
	LuaObject obj(state, LUA_GLOBALSINDEX);
	CHECK(obj.IsTable());
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_CopyConstructorFromLuaStackObject)
{
	LuaStateOwner state;
	LuaObject obj(state->GetGlobals_Stack());
	CHECK(obj.IsTable());
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_CopyConstructorFromLuaObject)
{
	LuaStateOwner state;
	LuaObject obj1(state->GetGlobals());
	LuaObject obj2(obj1);
	CHECK(state->Equal(obj1, obj2));
	CHECK(obj1.IsTable());
	CHECK(state->Equal(obj1, LuaObject(state->GetGlobals_Stack())));
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_AssignmentOperatorFromLuaObject)
{
	LuaStateOwner state;
	LuaObject obj1 = state->GetGlobals();		// Copy constructor
	LuaObject obj2;
	obj2 = obj1;								// Assignment operator
	CHECK(state->Equal(obj1, obj2));
	CHECK(obj1.IsTable());
	CHECK(state->Equal(obj1, LuaObject(state->GetGlobals_Stack())));
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_AssignmentOperatorFromLuaStackObject)
{
	LuaStateOwner state;
    LuaStackObject obj1 = state->GetGlobals_Stack();
	LuaObject obj2;
	obj2 = obj1;								// Assignment operator
	CHECK(state->Equal(LuaObject(obj1), obj2));
	CHECK(obj2.IsTable());
	CHECK(state->Equal(obj2, state->GetGlobals()));
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_Casts)
{
	LuaStateOwner state;
	LuaObject obj(state);

	// operator casts
//	LuaState* var = obj.GetState();
//	CHECK(var == obj.m_state);

//	lua_State* var2 = obj.GetCState();
//	CHECK(var2 == obj.m_state->m_state);

//	var = obj.GetState();
//	CHECK(var == obj.m_state);

//	var2 = obj.GetCState();
//	CHECK(var2 == obj.m_state->m_state);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_Assign)
{
	LuaStateOwner state;
	LuaObject obj(state);
	CHECK(obj.Type() == LUA_TNIL);
	CHECK(obj.IsNil());
	CHECK(strcmp(obj.TypeName(), "nil") == 0);

	obj.AssignNil(state);
	CHECK(obj.Type() == LUA_TNIL);
	CHECK(obj.IsNil());
	CHECK(strcmp(obj.TypeName(), "nil") == 0);

	obj.AssignBoolean(state, true);
	CHECK(obj.Type() == LUA_TBOOLEAN);
	CHECK(obj.IsBoolean());
	CHECK(obj.GetBoolean() == true);
	CHECK(strcmp(obj.TypeName(), "boolean") == 0);

	obj.AssignNumber(state, 5.5);
	CHECK(obj.Type() == LUA_TNUMBER);
	CHECK(obj.IsNumber());
	CHECK(obj.GetNumber() == 5.5);
	CHECK(obj.GetInteger() == 5);		// Should downcast.
	CHECK(strcmp(obj.TypeName(), "number") == 0);

	obj.AssignString(state, "Hello");
	CHECK(obj.Type() == LUA_TSTRING);
	CHECK(obj.IsString());
	CHECK(strcmp(obj.GetString(), "Hello") == 0);
	CHECK(strcmp(obj.TypeName(), "string") == 0);

	lua_WChar helloStr[] = { 'H', 'e', 'l', 'l', 'o', 0 };
	obj.AssignWString(state, helloStr);
	CHECK(obj.Type() == LUA_TWSTRING);
	CHECK(obj.IsWString());
	CHECK(lp_wcscmp(obj.GetWString(), helloStr) == 0);
	CHECK(strcmp(obj.TypeName(), "wstring") == 0);

/*	obj.AssignUserData(state, (void*)0x12345678);
	CHECK(obj.Type() == LUA_TUSERDATA);
	CHECK(obj.IsUserData());
	CHECK(!obj.IsLightUserData());
	CHECK(obj.GetUserData() == (void*)0x12345678);
	CHECK(strcmp(obj.TypeName(), "userdata") == 0);
*/
	obj.AssignLightUserData(state, (void*)0x12345678);
	CHECK(obj.Type() == LUA_TLIGHTUSERDATA);
	CHECK(obj.IsUserData());
	CHECK(obj.IsLightUserData());
	CHECK(obj.GetUserData() == (void*)0x12345678);
	CHECK(strcmp(obj.TypeName(), "userdata") == 0);

	// AssignObject test
	LuaObject obj2(state);
	obj2.AssignNumber(state, 6.0);
	CHECK(obj2.IsNumber()  &&  obj2.GetNumber() == 6.0);

	obj.AssignObject(obj2);
	CHECK(obj.Type() == LUA_TNUMBER);
	CHECK(obj.IsNumber());
	CHECK(obj.GetNumber() == 6.0);
	CHECK(obj == obj2);
	CHECK(strcmp(obj.TypeName(), "number") == 0);

	// AssignTable test
	obj.AssignNewTable(state);
	CHECK(obj.Type() == LUA_TTABLE);
	CHECK(obj.IsTable());
	CHECK(strcmp(obj.TypeName(), "table") == 0);

	// Can't test AssignTObject here.
}


//////////////////////////////////////////////////////////////////////////
#if 0

TEST(LuaObject_SetTable_Array)
{
	LuaStateOwner state;
	LuaObject obj;
	obj.AssignNewTable(state);
	CHECK(obj.IsTable());

	for (int i = 0; i < 500; ++i)
	{
		obj.SetNumber(i, i * 2);
	}

	CHECK(obj.GetTableCount() == 500);	// This is slow, because it has to count all items.
//jj	CHECK(obj.GetN() == 499);		// Why?  Lua is 1-based and GetN() starts counting
		// from 1 and then only for contiguous entries.

	// Remove an entry from the middle of the array.
	obj.SetNil(250);
	CHECK(obj.GetTableCount() == 499);
//jj	CHECK(obj.GetN() == 249);		// Why is this 249?  There are actually 499
		// entries in the table, but LuaObject::GetN() only counts the
		// contiguous entries up to the first nil it finds.

	// Remove the element 250 instead.
	obj.Remove(250);
	CHECK(obj.GetTableCount() == 498);
//jj	CHECK(obj.GetN() == 248);		// Ack, 248?  Yep.  LuaObject::Remove(),
		// which internally calls table.remove(), just decrements the current
		// N by 1.  It didn't pay attention to the fact that we just removed
		// the nil value, thereby making there be 498 contiguous entries.
		// We have to correct for it with a SetN() call.

	// Set the correct count.
//	obj.SetN(498);
	CHECK(obj.GetTableCount() == 498);
//jj	CHECK(obj.GetN() == 498);

	// Insert some elements.
	int tableCount1 = obj.GetTableCount();
	CHECK(tableCount1 == 498);
	LuaObject tempObj;
	tempObj.AssignNewTable(state);
	obj.Insert(100, tempObj);
	int tableCount2 = obj.GetTableCount();
	CHECK(tableCount1 == 498);
	obj.Insert(101, tempObj);
	CHECK(obj.GetTableCount() == 499);
//jj	CHECK(obj.GetN() == 500);

	// Verify the inserted elements are tables.
	LuaObject verifyObj = obj[100];
	CHECK(verifyObj.IsTable());
	CHECK(verifyObj == tempObj);

	verifyObj = obj[101];
	CHECK(verifyObj.IsTable());
	CHECK(verifyObj == tempObj);

	verifyObj = obj[102];
	CHECK(!verifyObj.IsTable());
}

#endif

//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_SetTable_Array_With_Sort)
{
	LuaStateOwner state(true);
	LuaObject obj;
	obj.AssignNewTable(state);
	CHECK(obj.IsTable());

	for (int i = 1; i <= 500; ++i)
	{
		obj.SetNumber(i, 500 - i * 2);
	}

//	obj.SetN(500);
	obj.Sort();

	for (int i = 1; i <= 500; ++i)
	{
		LuaObject numObj = obj[i];
		CHECK(numObj.IsNumber());
		int num = numObj.GetInteger();
		CHECK(num == -(502 - i * 2));
	}
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_SetNil)
{
	LuaStateOwner state;
	LuaObject obj;
	obj.AssignNewTable(state);
	CHECK(obj.IsTable());

	// SetNil(number)
	obj.SetNumber(1, 5);
	CHECK(obj[1].GetNumber() == 5);
	obj.SetNil(1);
	CHECK(obj[1].IsNil());

	// SetNil(string)
	obj.SetNumber("Hello", 6);
	CHECK(obj["Hello"].GetNumber() == 6);
	obj.SetNil("Hello");
	CHECK(obj["Hello"].IsNil());

	// SetNil(object)
	LuaObject stringObj(state);
	stringObj.AssignString(state, "Test");
	obj.SetNumber(stringObj, 7);
	CHECK(obj[stringObj].GetNumber() == 7);
	obj.SetNil(stringObj);
	CHECK(obj[stringObj].IsNil());
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_SetBoolean)
{
	LuaStateOwner state;
	LuaObject obj(state);
	obj.AssignNewTable(state);
	CHECK(obj.IsTable());

	// number
	obj.SetBoolean(1, true);
	CHECK(obj[1].GetBoolean() == true);

	// string
	obj.SetBoolean("Hello", false);
	CHECK(obj["Hello"].GetBoolean() == false);

	// object
	LuaObject stringObj(state);
	stringObj.AssignString(state, "Test");
	obj.SetBoolean(stringObj, true);
	CHECK(obj[stringObj].GetBoolean() == true);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_SetNumber)
{
	LuaStateOwner state;
	LuaObject obj(state);
	obj.AssignNewTable(state);
	CHECK(obj.IsTable());

	// number
	obj.SetNumber(1, 5);
	CHECK(obj[1].GetNumber() == 5);

	// string
	obj.SetNumber("Hello", 6);
	CHECK(obj["Hello"].GetNumber() == 6);

	// object
	LuaObject stringObj(state);
	stringObj.AssignString(state, "Test");
	obj.SetNumber(stringObj, 7);
	CHECK(obj[stringObj].GetNumber() == 7);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_SetString)
{
	LuaStateOwner state;
	LuaObject obj(state);
	obj.AssignNewTable(state);
	CHECK(obj.IsTable());

	// number
	obj.SetString(1, "Test1");
	CHECK(strcmp(obj[1].GetString(), "Test1") == 0);

	// string
	obj.SetString("Hello", "Test2");
	CHECK(strcmp(obj["Hello"].GetString(), "Test2") == 0);

	// object
	LuaObject stringObj(state);
	stringObj.AssignString(state, "Test");
	obj.SetString(stringObj, "Test3");
	CHECK(strcmp(obj[stringObj].GetString(), "Test3") == 0);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_SetWString)
{
	LuaStateOwner state;
	LuaObject obj(state);
	obj.AssignNewTable(state);
	CHECK(obj.IsTable());

	// number
	lua_WChar test1Str[] = { 'T', 'e', 's', 't', '1', 0 };
	obj.SetWString(1, test1Str);
	CHECK(lp_wcscmp(obj[1].GetWString(), test1Str) == 0);

	// string
	lua_WChar test2Str[] = { 'T', 'e', 's', 't', '2', 0 };
	obj.SetWString("Hello", test2Str);
	CHECK(lp_wcscmp(obj["Hello"].GetWString(), test2Str) == 0);

	// object
	LuaObject stringObj(state);
	stringObj.AssignString(state, "Test");

	lua_WChar test3Str[] = { 'T', 'e', 's', 't', '3', 0 };
	obj.SetWString(stringObj, test3Str);
	CHECK(lp_wcscmp(obj[stringObj].GetWString(), test3Str) == 0);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_SetUserData)
{
	LuaStateOwner state;
	LuaObject obj(state);
	obj.AssignNewTable(state);
	CHECK(obj.IsTable());
/*
	// number
	obj.SetUserData(1, (void*)0x12345678);
	CHECK(obj[1].GetUserData() == (void*)0x12345678);

	// string
	obj.SetUserData("Hello", (void*)0x87654321);
	CHECK(obj["Hello"].GetUserData() == (void*)0x87654321);

	// object
	LuaObject stringObj(state);
	stringObj.AssignString(state, "Test");
	obj.SetUserData(stringObj, (void*)0x02468024);
	CHECK(obj[stringObj].GetUserData() == (void*)0x02468024);
*/
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_SetLightUserData)
{
	LuaStateOwner state;
	LuaObject obj(state);
	obj.AssignNewTable(state);
	CHECK(obj.IsTable());

	// number
	obj.SetLightUserData(1, (void*)0x12345678);
	CHECK(obj[1].GetUserData() == (void*)0x12345678);

	// string
	obj.SetLightUserData("Hello", (void*)0x87654321);
	CHECK(obj["Hello"].GetUserData() == (void*)0x87654321);

	// object
	LuaObject stringObj(state);
	stringObj.AssignString(state, "Test");
	obj.SetLightUserData(stringObj, (void*)0x02468024);
	CHECK(obj[stringObj].GetUserData() == (void*)0x02468024);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_SetObject)
{
	LuaStateOwner state;
	LuaObject obj(state);
	obj.AssignNewTable(state);
	CHECK(obj.IsTable());

	LuaObject testObj(state);

	// number
	testObj.AssignBoolean(state, true);
	obj.SetObject(1, testObj);
	CHECK(obj[1] == testObj);

	// string
	testObj.AssignNumber(state, 5);
	obj.SetObject("Hello", testObj);
	CHECK(obj["Hello"] == testObj);

	// object
	LuaObject stringObj(state);
	stringObj.AssignString(state, "Test");
	testObj.AssignString(state, "Stuff");
	obj.SetObject(stringObj, testObj);
	CHECK(obj[stringObj] == testObj);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_CreateTable)
{
	LuaStateOwner state;
	LuaObject globalsObj = state->GetGlobals();
	CHECK(globalsObj.IsTable());

	// number
	LuaObject obj = globalsObj.CreateTable(1);
	obj.SetNumber(500, 50);
	CHECK(globalsObj[1].IsTable());
	CHECK(globalsObj[1][500].IsNumber());
	CHECK(globalsObj[1][500].GetNumber() == 50);

	// string
	obj = globalsObj.CreateTable("Hello");
	obj.SetBoolean("MyStuff", true);
	CHECK(globalsObj["Hello"].IsTable());
	CHECK(globalsObj["Hello"]["MyStuff"].IsBoolean());
	CHECK(globalsObj["Hello"]["MyStuff"].GetBoolean() == true);

	// object
	LuaObject stringObj(state);
	stringObj.AssignString(state, "Test");

	obj = globalsObj.CreateTable(stringObj);
	obj.SetBoolean("OtherStuff", false);
	CHECK(globalsObj[stringObj].IsTable());
	CHECK(globalsObj[stringObj]["OtherStuff"].IsBoolean());
	CHECK(globalsObj[stringObj]["OtherStuff"].GetBoolean() == false);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_GetByFuncs)
{
	LuaStateOwner state;
	LuaObject obj(state);
	obj.AssignNewTable(state);
	CHECK(obj.IsTable());

	state->PushString("Hello");
    LuaStackObject stackObj = state->StackTop();

	LuaObject stringObj(state);
	stringObj.AssignString(state, "Test");

	obj.SetNumber(1, 5);
	obj.SetNumber("Hello", 6);
	obj.SetNumber(stringObj, 7);

	CHECK(obj[1].GetNumber() == 5);
	CHECK(obj["Hello"].GetNumber() == 6);
	CHECK(obj[stackObj].GetNumber() == 6);
	CHECK(obj[stringObj].GetNumber() == 7);
	CHECK(obj.GetByIndex(1).GetNumber() == 5);
	CHECK(obj.GetByName("Hello").GetNumber() == 6);
	CHECK(obj.GetByObject(stackObj).GetNumber() == 6);
	CHECK(obj.GetByObject(stringObj).GetNumber() == 7);

	// Now add a metatable to simulate a hierarchy.
	LuaObject baseObj;
	baseObj.AssignNewTable(state);
	baseObj.SetNumber(2, 8);
	baseObj.SetNumber("Hello2", 9);
	stringObj.AssignString(state, "Test2");
	baseObj.SetNumber(stringObj, 10);

	LuaObject metaTableObj;
	metaTableObj.AssignNewTable(state);
	metaTableObj.SetObject("__index", baseObj);
	obj.SetMetaTable(metaTableObj);

	// Now do the checks.
	CHECK(obj.RawGetByIndex(2).IsNil());
	CHECK(obj.RawGetByName("Hello2").IsNil());
	CHECK(obj.RawGetByObject(stringObj).IsNil());
	CHECK(obj.GetByIndex(2).GetNumber() == 8);
	CHECK(obj.GetByName("Hello2").GetNumber() == 9);
	CHECK(obj.GetByObject(stringObj).GetNumber() == 10);
	CHECK(obj[2].IsNumber());
	CHECK(obj[2].GetNumber() == 8);
	CHECK(obj["Hello2"].IsNumber());
	CHECK(obj["Hello2"].GetNumber() == 9);
	CHECK(obj[stringObj].IsNumber());
	CHECK(obj[stringObj].GetNumber() == 10);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_StrLen)
{
	LuaStateOwner state;
	LuaObject stringObj(state);
	stringObj.AssignString(state, "Test");
	CHECK(stringObj.StrLen() == 4);

	lua_WChar wideString[] = { 'W', 'i', 'd', 'e', ' ', 'S', 't', 'r', 'i', 'n', 'g', 0 };
	stringObj.AssignWString(state, wideString);
	CHECK(stringObj.StrLen() == 11);
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_PushStack)
{
	LuaStateOwner state;
	LuaObject stringObj(state);
	stringObj.AssignString(state, "Test");
	stringObj.Push();
    LuaStackObject stackObj = state->StackTop();
	CHECK(state->GetTop() == 1);
	stringObj.Push();
    LuaStackObject stack2Obj = state->StackTop();
	CHECK(state->GetTop() == 2);
	CHECK(stackObj == stack2Obj);
	CHECK(stackObj.IsString());
	CHECK(strcmp(stackObj.GetString(), "Test") == 0);
	state->Pop();
	CHECK(state->GetTop() == 1);
}


//////////////////////////////////////////////////////////////////////////
static int LS_AddPrint(LuaState* state)
{
	LuaStack args(state);
	assert(args[1].IsNumber());
	assert(args[2].IsNumber());
	assert(args[3].IsString());

	lua_Number add = args[1].GetNumber() + args[2].GetNumber();
	printf("%f_%s\n", add, args[3].GetString());

	return 0;
}


static int LS_Add(LuaState* state)
{
	LuaStack args(state);
	assert(args[1].IsNumber());
	assert(args[2].IsNumber());

	lua_Number add = args[1].GetNumber() + args[2].GetNumber();
	state->PushNumber(add);

	return 1;
}

class TestObject
{
public:
	TestObject(float startNumber) :
		m_startNumber(startNumber)
	{
	}

	int LS_Mul(LuaState* state)
	{
		LuaStack args(state);
		assert(args[1].IsNumber());
		assert(args[2].IsNumber());

		lua_Number value = m_startNumber + args[1].GetNumber() * args[2].GetNumber();
		state->PushNumber(value);

		return 1;
	}

protected:
	float m_startNumber;
};


//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_PCall_FunctionCallNoReturnValue)
{
	LuaStateOwner state;
	state->GetGlobals().Register("AddPrint", LS_AddPrint);

	LuaObject addPrintObj = state->GetGlobals()["AddPrint"];
	CHECK(addPrintObj.IsFunction());
	CHECK(addPrintObj.IsCFunction());

	int top = state->GetTop();
	LuaCall call = addPrintObj;
	call << 5 << 10 << "Hello" << LuaRun();
	CHECK(top == state->GetTop());
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_PCall_FunctionCallWithReturnValue)
{
	LuaStateOwner state;
	state->GetGlobals().Register("Add", LS_Add);

	LuaObject addObj = state->GetGlobals()["Add"];
	CHECK(addObj.IsFunction());
	CHECK(addObj.IsCFunction());

	int top = state->GetTop();
	{
		LuaAutoBlock autoBlock(state);
		LuaCall call = addObj;
		LuaStackObject resultObj = call << 5 << 10 << LuaRun();
		CHECK(resultObj.IsNumber());
		CHECK(resultObj.GetNumber() == 15);
		CHECK(top + 1 == state->GetTop());
	}

	CHECK(top == state->GetTop());

	// Now, don't accept the return value.
	{
		LuaAutoBlock autoBlock(state);
		LuaCall call = addObj;
		LuaStackObject resultObj = call << 5 << 10 << LuaRun();
		CHECK(resultObj.IsNumber());
		CHECK(top == state->GetTop() - 1);
	}
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_FunctionCallNoReturnValue)
{
	LuaStateOwner state;
	state->GetGlobals().Register("AddPrint", LS_AddPrint);

	LuaObject addPrintObj = state->GetGlobals()["AddPrint"];
	CHECK(addPrintObj.IsFunction());
	CHECK(addPrintObj.IsCFunction());

	int top = state->GetTop();
	LuaCall call = addPrintObj;
	call << 5 << 10 << "Hello" << LuaRun();
	CHECK(top == state->GetTop());
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_FunctionCallWithReturnValue)
{
	LuaStateOwner state;
	state->GetGlobals().Register("Add", LS_Add);

	LuaObject addObj = state->GetGlobals()["Add"];
	CHECK(addObj.IsFunction());
	CHECK(addObj.IsCFunction());

	int top = state->GetTop();
	{
		LuaAutoBlock autoBlock(state);
		LuaCall call = addObj;
		LuaStackObject resultObj = call << 5 << 10 << LuaRun();
		CHECK(resultObj.IsNumber());
		CHECK(resultObj.GetNumber() == 15);
		CHECK(top + 1 == state->GetTop());
	}

	CHECK(top == state->GetTop());

	// Now, don't accept the return value.
	{
		LuaAutoBlock autoBlock(state);
		LuaCall call = addObj;
		LuaStackObject resultObj = call << 5 << 10 << LuaRun(0);
		CHECK(resultObj.IsNone());
		CHECK(top == state->GetTop());
	}
}


//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_DirectFunctionCall)
{
	LuaStateOwner state;
	state->GetGlobals().Register("Add", LS_Add);
	state->GetGlobals().Register("AddPrint", LS_AddPrint);
	
	TestObject obj(5);
	state->GetGlobals().Register("Mul", obj, &TestObject::LS_Mul);

	LuaObject addObj = state->GetGlobals()["Add"];
	LuaFunction<int> addFunction = addObj;
	LuaObject addPrintObj = state->GetGlobals()["AddPrint"];
	LuaFunction<void> addPrintFunction = addPrintObj;
	LuaObject mulObj = state->GetGlobals()["Mul"];
	LuaFunction<float> mulFunction = mulObj;

	int top = state->GetTop();
	int result = addFunction(5, 10);
	CHECK(result == 15);
	int afterTop = state->GetTop();
	CHECK(top == afterTop);

	addPrintFunction(20, 30, "My String");

	top = state->GetTop();
	float fresult = mulFunction(5, 10);
	CHECK(fresult == 5 + 5 * 10);
	afterTop = state->GetTop();
	CHECK(top == afterTop);
}


//////////////////////////////////////////////////////////////////////////
int DFR_Add(int num1, int num2)
{
	return num1 + num2;
}

void DFR_AddPrint(float num1, float num2, const char* string)
{
	float add = num1 + num2;
	printf("%f_%s\n", add, string);
}

class DFRObject
{
public:
	DFRObject(float startValue) :
		m_startValue(startValue)
	{
	}

	float Mul(float num1, float num2)
	{
		return m_startValue + num1 * num2;
	}

protected:
	float m_startValue;
};

TEST(LuaObject_DirectFunctionRegister)
{
	LuaStateOwner state;
	state->GetGlobals().RegisterDirect("Add", DFR_Add);
	state->GetGlobals().RegisterDirect("AddPrint", DFR_AddPrint);

	DFRObject obj(5);
	state->GetGlobals().RegisterDirect("Mul", obj, &DFRObject::Mul);

	LuaObject addObj = state->GetGlobals()["Add"];
	LuaFunction<int> addFunction = addObj;
	LuaObject addPrintObj = state->GetGlobals()["AddPrint"];
	LuaFunction<void> addPrintFunction = addPrintObj;
	LuaObject mulObj = state->GetGlobals()["Mul"];
	LuaFunction<float> mulFunction = mulObj;
	
	int top = state->GetTop();
	int result = addFunction(5, 10);
	CHECK(result == 15);
	int afterTop = state->GetTop();
	CHECK(top == afterTop);

	addPrintFunction(20.2, 30.5, "My String");

	top = state->GetTop();
	float fresult = mulFunction(5, 10);
	CHECK(fresult == 5 + 5 * 10);
	afterTop = state->GetTop();
	CHECK(top == afterTop);
}


template <typename Func>
inline size_t FuncSize(Func)
{
	return sizeof(Func);
}


template <typename Callee, typename Func>
inline size_t CalleeFuncSize(const Callee&, Func)
{
	return sizeof(Callee*) + sizeof(Func);
}


static size_t LS_FuncSize(LuaState* state)
{
	LuaObject obj = LuaStackObject(state, lua_upvalueindex(1));
	return obj.StrLen();
}


class FuncSizeObject
{
public:
	FuncSizeObject()
	{
	}

	size_t LS_CalleeFuncSize(LuaState* state)
	{
		LuaObject obj(state, lua_upvalueindex(1));
		return obj.StrLen();
	}

protected:
	char buffer[1000];
};

TEST(LuaObject_FunctorSizeTest)
{
	LuaStateOwner state;
	state->GetGlobals().RegisterDirect("Func1", LS_FuncSize);

	LuaObject obj = state->GetGlobals()["Func1"];
    LuaCall callObj(obj);
	LuaStackObject retObj = callObj << LuaRun();
	size_t returnedSize = retObj.GetInteger();
	CHECK(returnedSize == FuncSize(LS_FuncSize));

	FuncSizeObject theObject;
	state->GetGlobals().RegisterDirect("Func2", theObject, &FuncSizeObject::LS_CalleeFuncSize);
	LuaObject func2Obj = state->GetGlobals()["Func2"];
	callObj = func2Obj;
	retObj = callObj << LuaRun();
	returnedSize = retObj.GetInteger();
	CHECK(returnedSize == CalleeFuncSize(theObject, &FuncSizeObject::LS_CalleeFuncSize));
}

	
//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_CheckConvertibleTypes)
{
	LuaStateOwner state;
	LuaObject globalsObj = state->GetGlobals();
	globalsObj.SetNumber("Number", 501);

	LuaObject numObj = globalsObj["Number"];
	CHECK(numObj.IsNumber());
	CHECK(!numObj.IsString());
	CHECK(!numObj.IsWString());
	CHECK(numObj.IsConvertibleToNumber());
	CHECK(numObj.IsConvertibleToString());
	CHECK(numObj.IsConvertibleToWString());

	lua_Number num = numObj.ToNumber();
	CHECK(numObj.IsNumber());

	const char* str = numObj.ToString();
	CHECK(numObj.IsString());
	CHECK(strcmp(str, "501") == 0);
	CHECK(numObj.IsConvertibleToNumber());
	CHECK(!numObj.IsConvertibleToWString());

	num = numObj.ToNumber();
	CHECK(numObj.IsString());		// Once in string form, it stays there.

	globalsObj.SetNumber("Number", 501);
	numObj = globalsObj["Number"];

	const lua_WChar* wstr = numObj.ToWString();
	CHECK(numObj.IsWString());
	lua_WChar num501[] = { '5', '0', '1', 0 };
	CHECK(lp_wcscmp(wstr, num501) == 0);
	CHECK(numObj.IsConvertibleToNumber());
	CHECK(!numObj.IsConvertibleToString());

	num = numObj.ToNumber();
	CHECK(numObj.IsWString());		// Again, once in string form, it stays there.

	globalsObj.SetNumber("Number", 501);
	numObj = globalsObj["Number"];

	int len = numObj.ToStrLen();
	CHECK(len == 3);
	CHECK(numObj.IsString());
}

	
//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_MetaTable)
{
	LuaStateOwner state;

	// Make a base object.
	LuaObject baseObj;
	baseObj.AssignNewTable(state);
	baseObj.SetString("base", "the base");
	baseObj.SetObject("__index", baseObj);

	// Make a table for the metatable.
	LuaObject metaTableObj(state);
	metaTableObj.AssignNewTable(state);
	
	// Set up the indexing.
	LuaObject stringObj(state);
	stringObj.AssignString(state, "Test");
	metaTableObj.SetString(1, "MyLittleNumber");
	metaTableObj.SetString("__name", "MyLittleMetaTable");
	metaTableObj.SetString(stringObj, "MyLittleObject");
	
	// Make the metatable be the next lookup index.
	metaTableObj.SetObject("__index", metaTableObj);

	// Create a global called MyTable and assign the metatable.
	LuaObject myTableObj = state->GetGlobals().CreateTable("MyTable");
	myTableObj.SetMetaTable(metaTableObj);

	// Look it up again.
	LuaObject testTableObj = state->GetGlobals()["MyTable"];
	LuaObject testMetaTableObj = testTableObj.GetMetaTable();
	CHECK(metaTableObj == testMetaTableObj);
	CHECK(metaTableObj["__name"].IsString());
	CHECK(strcmp(metaTableObj["__name"].GetString(), "MyLittleMetaTable") == 0);

	// Test GetBy*() functions
	CHECK(testTableObj.GetByIndex(1).IsString());
	CHECK(strcmp(testTableObj.GetByIndex(1).GetString(), "MyLittleNumber") == 0);
	CHECK(testTableObj.GetByName("__name").IsString());
	CHECK(strcmp(testTableObj.GetByName("__name").GetString(), "MyLittleMetaTable") == 0);
	CHECK(testTableObj.GetByObject(stringObj).IsString());
	CHECK(strcmp(testTableObj.GetByObject(stringObj).GetString(), "MyLittleObject") == 0);
	CHECK(testTableObj.GetByIndex(2).IsNil());

	// Test Get() functions reaching the baseObj.
	CHECK(testTableObj.Get("base").IsNil());
	metaTableObj.SetMetaTable(baseObj);
	CHECK(testTableObj.Get("base").IsString());
	CHECK(strcmp(testTableObj.GetByName("base").GetString(), "the base") == 0);

	// Test operator functions
	CHECK(testTableObj[1].IsString());
	CHECK(strcmp(testTableObj[1].GetString(), "MyLittleNumber") == 0);
	CHECK(testTableObj["__name"].IsString());
	CHECK(strcmp(testTableObj["__name"].GetString(), "MyLittleMetaTable") == 0);
	CHECK(testTableObj[stringObj].IsString());
	CHECK(strcmp(testTableObj[stringObj].GetString(), "MyLittleObject") == 0);

	// Test operator functions
	CHECK(testTableObj.RawGet(1).IsNil());
	CHECK(testTableObj.RawGet("__name").IsNil());
	CHECK(testTableObj.RawGet(stringObj).IsNil());
}


//////////////////////////////////////////////////////////////////////////
static int CallbackFunction(LuaState* state)
{
	// For purposes of getting locals.
	return 0;
}


TEST(LuaState_Callback)
{
	LuaStateOwner state;

	state->GetGlobals().Register("Callback", CallbackFunction);
	state->DoString("Callback()");
}


static int WriteToBinaryFile(lua_State* L, const void* p, size_t sz, void* ud)
{
	FILE* file = (FILE*)ud;
	fwrite(p, sz, 1, file);
	return 0;
}


TEST(LuaState_LoadCompiledScript)
{
	{
		LuaStateOwner state;
		
		int ret = state->LoadFile("CompileMe.lua");
		CHECK(ret == 0);

		CHECK(state->GetTop() == 1);

		FILE* file = fopen("CompileMe.lc", "wb");
		state->Dump(WriteToBinaryFile, file, 1, '=');
		fclose(file);

		CHECK(state->GetTop() == 1);
		state->Pop();
	}

#if 0

	{
		LuaStateOwner state;

		int ret = state->DoFile("CompileMe.lc");
		CHECK(ret == 0);

		LuaObject wstrObj = state->GetGlobals()["wstr"];
		CHECK(wstrObj.IsWString());

		const lua_WChar* wstr = wstrObj.GetWString();
		lua_WChar wideCharacterString[] = { 'W', 'i', 'd', 'e', ' ', 'c', 'h', 'a', 'r', 'a', 'c', 't', 'e', 'r', ' ', 's', 't', 'r', 'i', 'n', 'g', 0 };
		CHECK(lp_wcscmp(wstr, wideCharacterString) == 0);
	}

#endif
}


TEST(LuaState_LoadString)
{
	LuaStateOwner state;

	int ret = state->LoadString("MyTable = { Name1 = 5, Name2 = 10 Name3 = 15 }");
	CHECK(ret == LUA_ERRSYNTAX);

	ret = state->LoadString("MyTable = { Name1 = 5, Name2 = 10, Name3 = 15 }");
	CHECK(ret == 0);

	ret = state->PCall(0, LUA_MULTRET, 0);
	CHECK(ret == 0);

	CHECK(state->GetGlobals()["MyTable"].IsTable());

	LuaObject myTableObj = state->GetGlobals()["MyTable"];
	CHECK(myTableObj["Name1"].IsNumber());
	CHECK(myTableObj["Name1"].GetNumber() == 5);
}

/*
TEST(LuaState_LoadWString)
{
	LuaStateOwner state;

	int ret = state->LoadWString(L"MyTable = { Name1 = 5, Name2 = 10 Name3 = 15 }");
	CHECK(ret == LUA_ERRSYNTAX);
	const char* errorMessage = state->Stack(-1).GetString();

	CHECK(state->GetTop() == 1);
	state->Pop();

	// Should work either way.
	ret = state->LoadWString(L"\xfeffMyTable = { Name1 = 5, Name2 = 10, Name3 = 15 }");
	CHECK(ret == 0);
	CHECK(state->GetTop() == 1);
	state->Pop();

	ret = state->LoadWString(L"MyTable = { Name1 = 5, Name2 = 10, Name3 = 15 }");
	CHECK(ret == 0);
	CHECK(state->GetTop() == 1);

	ret = state->PCall(0, LUA_MULTRET, 0);
	CHECK(ret == 0);
	CHECK(state->GetTop() == 0);

	CHECK(state->GetGlobals()["MyTable"].IsTable());

	LuaObject myTableObj = state->GetGlobals()["MyTable"];
	CHECK(myTableObj["Name1"].IsNumber());
	CHECK(myTableObj["Name1"].GetNumber() == 5);
}
*/

#if LUA_EXCEPTIONS

TEST(LuaState_ExceptionTest)
{
	LuaStateOwner state;
	state->DoString("MyTable = { 1, 2, 3, 4 }");

	bool hitException = false;

	try
	{
		void* data = state->GetGlobal("MyTable")[1].GetUserData();  (void)data;
	}
	catch (const LuaException& /*e*/)
	{
		hitException = true;
	}

	CHECK(hitException == true);

	hitException = false;

	try
	{
		LuaObject obj = state->GetGlobals()["MyTable"][1][2];
	}
	catch (const LuaException& /*e*/)
	{
		hitException = true;
	}

	CHECK(hitException == true);
}

#endif // LUA_EXCEPTIONS


TEST(LuaState_BitOperators)
{
	LuaStateOwner state(true);
	state->DoString("i1 = 10");
	state->DoString("i2 = 20; f1 = 15.5; f2 = .5");
	state->DoString("i3 = -10");

	CHECK(state->GetGlobals()["i1"].IsNumber());
	CHECK(state->GetGlobals()["i2"].IsNumber());
	CHECK(state->GetGlobals()["i3"].IsNumber());
	CHECK(state->GetGlobals()["f1"].IsNumber());
	CHECK(state->GetGlobals()["f2"].IsNumber());

	state->DoString("res1 = i1 + i2");
	CHECK(state->GetGlobals()["res1"].IsNumber());
	CHECK(state->GetGlobals()["res1"].GetNumber() == 30);

	state->DoString("res1 = i1 + f1");
	CHECK(state->GetGlobals()["res1"].IsNumber());
	CHECK(state->GetGlobals()["res1"].GetNumber() == 25.5);

	state->DoString("res1 = -i1");
	CHECK(state->GetGlobals()["res1"].IsNumber());
	CHECK(state->GetGlobals()["res1"].GetNumber() == -10);

	state->DoString("res1 = i1 | 3");
	CHECK(state->GetGlobals()["res1"].IsNumber());
	CHECK(state->GetGlobals()["res1"].GetNumber() == 11);

	state->DoString("res1 = i1 & 2");
	CHECK(state->GetGlobals()["res1"].IsNumber());
	CHECK(state->GetGlobals()["res1"].GetNumber() == 2);

	state->DoString("res1 = i1 ^^ 12");
	CHECK(state->GetGlobals()["res1"].IsNumber());
	CHECK(state->GetGlobals()["res1"].GetNumber() == 6);

	state->DoString("res1 = 1 << 4");
	CHECK(state->GetGlobals()["res1"].IsNumber());
	CHECK(state->GetGlobals()["res1"].GetNumber() == 16);

	state->DoString("res1 = 32 >> 4");
	CHECK(state->GetGlobals()["res1"].IsNumber());
	CHECK(state->GetGlobals()["res1"].GetNumber() == 2);

	state->DoString("i1 = 0x1000");
	CHECK(state->GetGlobals()["i1"].IsNumber());
	CHECK(state->GetGlobals()["i1"].GetNumber() == 0x1000);

	state->DoString("i1 = i1 | 0x80000000");
	CHECK(state->GetGlobals()["i1"].IsNumber());
	CHECK(state->GetGlobals()["i1"].GetNumber() == 0x80001000);
}


TEST(LuaPlus_TestANSIFile)
{
	LuaStateOwner state(true);
	int ret = state->DoFile("TestANSI.lua");
	CHECK(ret == 0);

	LuaObject sObj = state->GetGlobal("s");
	CHECK(sObj.IsString());
}


TEST(LuaPlus_TestUnicodeFile)
{
	LuaStateOwner state(true);
	int ret = state->DoFile("TestUnicode.lua");
	CHECK(ret == 0);

	LuaObject sObj = state->GetGlobal("s");
	CHECK(sObj.IsWString());
}


TEST(LuaPlus_BogusCharacters)
{
	LuaStateOwner state(true);
	int ret = state->LoadFile("BogusCharacters.lua");
	CHECK(ret == LUA_ERRSYNTAX);
}

//////////////////////////////////////////////////////////////////////////
TEST(LuaObject_LotsOTables)
{
	LuaStateOwner state;
	std::list<LuaObject> lotsOTables;

	for (size_t i = 0; i < 100000; ++i)
	{
		LuaObject tableObj;
		tableObj.AssignNewTable(state);
		tableObj.SetNumber("SomeNumber", i);  // Create newTable.SomeNumber = i;
		lotsOTables.push_back(tableObj);
	}

	size_t i = 0;
	for (std::list<LuaObject>::iterator it = lotsOTables.begin(); it != lotsOTables.end(); ++it, ++i)
	{
		LuaObject& tableObj = (*it);
		CHECK(tableObj.IsTable());
		LuaObject someNumberObj = tableObj["SomeNumber"];
		CHECK(someNumberObj.IsNumber());
		CHECK(someNumberObj.GetNumber() == i);
	}
}

//////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    return UnitTest::RunAllTests();
}

