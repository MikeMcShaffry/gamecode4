///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2010 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUASTATE_INL
#define LUASTATE_INL

#include <string.h>
#include <assert.h>

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus
{

LUAPLUS_INLINE lua_CFunction LuaState::AtPanic(lua_CFunction panicf)
{
	return lua_atpanic(LuaState_to_lua_State(this), panicf);
}

LUAPLUS_INLINE LuaStackObject LuaState::Stack(int index)
{
    return LuaStackObject(this, index);
}

LUAPLUS_INLINE LuaStackObject LuaState::StackTop()
{
    return LuaStackObject(this, GetTop());
}

// Basic stack manipulation.
LUAPLUS_INLINE int LuaState::GetTop()
{
	return lua_gettop(LuaState_to_lua_State(this));
}

LUAPLUS_INLINE void LuaState::SetTop(int index)
{
	lua_settop(LuaState_to_lua_State(this), index);
}

LUAPLUS_INLINE void LuaState::PushValue(int index)
{
	lua_pushvalue(LuaState_to_lua_State(this), index);
}

LUAPLUS_INLINE void LuaState::PushValue(LuaStackObject& object)
{
	lua_pushvalue(LuaState_to_lua_State(this), object);
}

LUAPLUS_INLINE void LuaState::Remove(int index)
{
	lua_remove(LuaState_to_lua_State(this), index);
}

LUAPLUS_INLINE void LuaState::Insert(int index)
{
	lua_insert(LuaState_to_lua_State(this), index);
}

LUAPLUS_INLINE void LuaState::Replace(int index)
{
	lua_replace(LuaState_to_lua_State(this), index);
}


LUAPLUS_INLINE int LuaState::CheckStack(int size)
{
	return lua_checkstack(LuaState_to_lua_State(this), size);
}


LUAPLUS_INLINE void LuaState::XMove(LuaState* to, int n)
{
	lua_xmove(LuaState_to_lua_State(this), LuaState_to_lua_State(to), n);
}

	
// access functions (stack -> C)
LUAPLUS_INLINE int LuaState::IsNumber(int index) const {
	return lua_isnumber(LuaState_to_lua_State(this), index);
}


LUAPLUS_INLINE int LuaState::IsString(int index) const {
	return lua_isstring(LuaState_to_lua_State(this), index);
}


#if LUA_WIDESTRING

LUAPLUS_INLINE int LuaState::IsWString(int index) const {
	return lua_iswstring(LuaState_to_lua_State(this), index);
}

#endif /* LUA_WIDESTRING */


LUAPLUS_INLINE int LuaState::IsCFunction(int index) const {
	return lua_iscfunction(LuaState_to_lua_State(this), index);
}


LUAPLUS_INLINE int LuaState::IsUserData(int index) const {
	return lua_isuserdata(LuaState_to_lua_State(this), index);
}


LUAPLUS_INLINE int LuaState::IsFunction(int index) const {
	return lua_isfunction(LuaState_to_lua_State(this), index);
}


LUAPLUS_INLINE int LuaState::IsTable(int index) const {
	return lua_istable(LuaState_to_lua_State(this), index);
}


LUAPLUS_INLINE int LuaState::IsLightUserData(int index) const {
	return lua_islightuserdata(LuaState_to_lua_State(this), index);
}


LUAPLUS_INLINE int LuaState::IsNil(int index) const {
	return lua_isnil(LuaState_to_lua_State(this), index);
}


LUAPLUS_INLINE int LuaState::IsBoolean(int index) const {
	return lua_isboolean(LuaState_to_lua_State(this), index);
}


LUAPLUS_INLINE int LuaState::IsThread(int index) const {
	return lua_isthread(LuaState_to_lua_State(this), index);
}


LUAPLUS_INLINE int LuaState::IsNone(int index) const {
	return lua_isnone(LuaState_to_lua_State(this), index);
}


LUAPLUS_INLINE int LuaState::IsNoneOrNil(int index) const {
	return lua_isnoneornil(LuaState_to_lua_State(this), index);
}


LUAPLUS_INLINE int LuaState::Type(int index) const {
	return lua_type(LuaState_to_lua_State(this), index);
}


LUAPLUS_INLINE const char* LuaState::TypeName(int type) {
	return lua_typename(LuaState_to_lua_State(this), type);
}


LUAPLUS_INLINE int LuaState::Equal(int index1, int index2)
{
	return lua_equal(LuaState_to_lua_State(this), index1, index2);
}

LUAPLUS_INLINE int LuaState::RawEqual(int index1, int index2)
{
	return lua_rawequal(LuaState_to_lua_State(this), index1, index2);
}

LUAPLUS_INLINE int LuaState::LessThan(int index1, int index2)
{
	return lua_lessthan(LuaState_to_lua_State(this), index1, index2);
}


LUAPLUS_INLINE lua_Number LuaState::ToNumber(int index) {
	return lua_tonumber(LuaState_to_lua_State(this), index);
}


LUAPLUS_INLINE lua_Integer LuaState::ToInteger(int index) {
	return lua_tointeger(LuaState_to_lua_State(this), index);
}


LUAPLUS_INLINE int LuaState::ToBoolean(int index) {
	return lua_toboolean(LuaState_to_lua_State(this), index);
}


LUAPLUS_INLINE const char* LuaState::ToLString(int index, size_t* len) {
	return lua_tolstring(LuaState_to_lua_State(this), index, len);
}


LUAPLUS_INLINE const char* LuaState::ToString(int index) {
	return lua_tostring(LuaState_to_lua_State(this), index);
}


#if LUA_WIDESTRING

LUAPLUS_INLINE const lua_WChar* LuaState::ToLWString(int index, size_t* len) {
	return lua_tolwstring(LuaState_to_lua_State(this), index, len);
}


LUAPLUS_INLINE const lua_WChar* LuaState::ToWString(int index) {
	return lua_towstring(LuaState_to_lua_State(this), index);
}

#endif /* LUA_WIDESTRING */


LUAPLUS_INLINE size_t LuaState::ObjLen(int index) {
	return lua_objlen(LuaState_to_lua_State(this), index);
}


LUAPLUS_INLINE lua_CFunction LuaState::ToCFunction(int index) {
	return lua_tocfunction(LuaState_to_lua_State(this), index);
}


LUAPLUS_INLINE void* LuaState::ToUserData(int index) {
	return lua_touserdata(LuaState_to_lua_State(this), index);
}


LUAPLUS_INLINE lua_State* LuaState::ToThread(int index) {
	return lua_tothread(LuaState_to_lua_State(this), index);
}


LUAPLUS_INLINE const void* LuaState::ToPointer(int index) {
	return lua_topointer(LuaState_to_lua_State(this), index);
}

	
// push functions (C -> stack)
LUAPLUS_INLINE LuaStackObject LuaState::PushNil()
{
	lua_pushnil(LuaState_to_lua_State(this));
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushNumber(lua_Number n)
{
	lua_pushnumber(LuaState_to_lua_State(this), n);
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushInteger(int n)
{
	lua_pushnumber(LuaState_to_lua_State(this), n);
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushLString(const char *s, size_t len)
{
	lua_pushlstring(LuaState_to_lua_State(this), s, len);
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushString(const char *s)
{
	lua_pushstring(LuaState_to_lua_State(this), s);
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}

#if LUA_WIDESTRING
LUAPLUS_INLINE LuaStackObject LuaState::PushLWString(const lua_WChar* s, size_t len)
{
	lua_pushlwstring(LuaState_to_lua_State(this), s, len);
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushWString(const lua_WChar* s)
{
	lua_pushwstring(LuaState_to_lua_State(this), s);
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}
#endif /* LUA_WIDESTRING */

LUAPLUS_INLINE LuaStackObject LuaState::PushCClosure(lua_CFunction fn, int n)
{
	lua_pushcclosure(LuaState_to_lua_State(this), fn, n);
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushCFunction(lua_CFunction f)
{
	lua_pushcclosure(LuaState_to_lua_State(this), f, 0);
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushBoolean(bool value)
{
	lua_pushboolean(LuaState_to_lua_State(this), value);
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushLightUserData(void* p)
{
	lua_pushlightuserdata(LuaState_to_lua_State(this), p);
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}

LUAPLUS_INLINE LuaStackObject LuaState::PushThread()
{
	lua_pushthread(LuaState_to_lua_State(this));
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}


// get functions (Lua -> stack)
LUAPLUS_INLINE void LuaState::GetTable(int index)
{
	lua_gettable(LuaState_to_lua_State(this), index);
}

LUAPLUS_INLINE void LuaState::GetField(int index, const char* key) {
	lua_getfield(LuaState_to_lua_State(this), index, key);
}

	
LUAPLUS_INLINE void LuaState::RawGet(int index)
{
	lua_rawget(LuaState_to_lua_State(this), index);
}

LUAPLUS_INLINE void LuaState::RawGetI(int index, int n)
{
	lua_rawgeti(LuaState_to_lua_State(this), index, n);
}

LUAPLUS_INLINE LuaStackObject LuaState::CreateTable(int narr, int nrec) {
	lua_createtable(LuaState_to_lua_State(this), narr, nrec);
	return LuaStackObject(*this, GetTop());
}


LUAPLUS_INLINE LuaStackObject LuaState::NewUserData(size_t size) {
	lua_newuserdata(LuaState_to_lua_State(this), size);
	return LuaStackObject(*this, GetTop());
}

LUAPLUS_INLINE LuaStackObject LuaState::GetMetaTable(int index)
{
	lua_getmetatable(LuaState_to_lua_State(this), index);
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}

LUAPLUS_INLINE LuaStackObject LuaState::GetFEnv(int index) {
	lua_getfenv(LuaState_to_lua_State(this), index);
	return LuaStackObject(this, lua_gettop(LuaState_to_lua_State(this)));
}

LUAPLUS_INLINE LuaStackObject LuaState::GetGlobals_Stack()
{
	return LuaStackObject(*this, LUA_GLOBALSINDEX);
}

LUAPLUS_INLINE LuaStackObject LuaState::GetGlobal_Stack(const char *name)
{
	lua_getglobal(LuaState_to_lua_State(this), name);  return LuaStackObject(*this, GetTop());
}

LUAPLUS_INLINE LuaStackObject LuaState::GetRegistry_Stack()
{
	return LuaStackObject(*this, LUA_REGISTRYINDEX);  //{  lua_getregistry(LuaState_to_lua_State(this));
}


// set functions(stack -> Lua)
LUAPLUS_INLINE void LuaState::SetTable(int index)
{
	lua_settable(LuaState_to_lua_State(this), index);
}

LUAPLUS_INLINE void LuaState::SetField(int index, const char* key) {
	lua_setfield(LuaState_to_lua_State(this), index, key);
}

LUAPLUS_INLINE void LuaState::RawSet(int index)
{
	lua_rawset(LuaState_to_lua_State(this), index);
}

LUAPLUS_INLINE void LuaState::RawSetI(int index, int n)
{
	lua_rawseti(LuaState_to_lua_State(this), index, n);
}

LUAPLUS_INLINE void LuaState::SetMetaTable(int index)
{
	lua_setmetatable(LuaState_to_lua_State(this), index);
}

LUAPLUS_INLINE void LuaState::SetFEnv(int index)
{
	lua_setfenv(LuaState_to_lua_State(this), index);
}

// `load' and `call' functions (load and run Lua code)
#if LUA_EXT_RESUMABLEVM

LUAPLUS_INLINE void LuaState::VCall(int nargs, int nresults, void *ctx) {
	lua_vcall(LuaState_to_lua_State(this), nargs, nresults, ctx);
}


LUAPLUS_INLINE int LuaState::VPCall(int nargs, int nresults, int errfunc, void *ctx) {
	return lua_vpcall(LuaState_to_lua_State(this), nargs, nresults, errfunc, ctx);
}

#endif


LUAPLUS_INLINE void LuaState::Call(int nargs, int nresults) {
	lua_call(LuaState_to_lua_State(this), nargs, nresults);
}


LUAPLUS_INLINE int LuaState::PCall(int nargs, int nresults, int errf) {
	return lua_pcall(LuaState_to_lua_State(this), nargs, nresults, errf);
}


LUAPLUS_INLINE int LuaState::CPCall(lua_CFunction func, void* ud)
{
	return lua_cpcall(LuaState_to_lua_State(this), func, ud);
}

LUAPLUS_INLINE int LuaState::Load(lua_Reader reader, void *dt, const char *chunkname)
{
	return lua_load(LuaState_to_lua_State(this), reader, dt, chunkname);
}

#if LUA_WIDESTRING
LUAPLUS_INLINE int LuaState::WLoad(lua_Reader reader, void *dt, const char *chunkname)
{
	return lua_wload(LuaState_to_lua_State(this), reader, dt, chunkname);
}
#endif /* LUA_WIDESTRING */

#if LUA_ENDIAN_SUPPORT

LUAPLUS_INLINE int LuaState::Dump(lua_Chunkwriter writer, void* data, int strip, char endian)
{
	return lua_dumpendian(LuaState_to_lua_State(this), writer, data, strip, endian);
}

#else

LUAPLUS_INLINE int LuaState::Dump(lua_Chunkwriter writer, void* data)
{
	return lua_dump(LuaState_to_lua_State(this), writer, data);
}

#endif /* LUA_ENDIAN_SUPPORT */

#if LUA_EXT_RESUMABLEVM

LUAPLUS_INLINE void* LuaState::VContext() {
	return lua_vcontext(LuaState_to_lua_State(this));
}


LUAPLUS_INLINE int LuaState::IContext() {
	return lua_icontext(LuaState_to_lua_State(this));
}


LUAPLUS_INLINE void LuaState::ICall(int nargs, int nresults, int ctx) {
	lua_icall(LuaState_to_lua_State(this), nargs, nresults, ctx);
}


LUAPLUS_INLINE int LuaState::IPCall(int nargs, int nresults, int errf, int ctx) {
	return lua_ipcall(LuaState_to_lua_State(this), nargs, nresults, errf);
}

#endif /* LUA_EXT_RESUMABLEVM */

LUAPLUS_INLINE int LuaState::LoadFile(const char* filename)
{
	return luaL_loadfile(LuaState_to_lua_State(this), filename);
}

LUAPLUS_INLINE int LuaState::DoFile(const char *filename)
{
	return luaL_dofile(LuaState_to_lua_State(this), filename);
}

LUAPLUS_INLINE int LuaState::DoString(const char *str)
{
	return luaL_dostring(LuaState_to_lua_State(this), str);
}

LUAPLUS_INLINE int LuaState::LoadBuffer(const char* buff, size_t size, const char* name)
{
	return luaL_loadbuffer(LuaState_to_lua_State(this), buff, size, name);
}

LUAPLUS_INLINE int LuaState::DoBuffer(const char *buff, size_t size, const char *name)
{
	return (luaL_loadbuffer(LuaState_to_lua_State(this), buff, size, name) || lua_pcall(LuaState_to_lua_State(this), 0, 0, 0));
}

#if LUA_WIDESTRING
LUAPLUS_INLINE int LuaState::DoWString(const lua_WChar *str, const char* name)
{
	(void)name;
	return luaL_dowstring(LuaState_to_lua_State(this), str);
}

LUAPLUS_INLINE int LuaState::LoadWBuffer(const lua_WChar* buff, size_t size, const char* name)
{
	return luaL_loadwbuffer(LuaState_to_lua_State(this), buff, size, name);
}

LUAPLUS_INLINE int LuaState::DoWBuffer(const lua_WChar* buff, size_t size, const char *name)
{
	return (luaL_loadwbuffer(LuaState_to_lua_State(this), buff, size, name) || lua_pcall(LuaState_to_lua_State(this), 0, 0, 0));
}
#endif /* LUA_WIDESTRING */

/*
** coroutine functions
*/
#if LUA_EXT_RESUMABLEVM

LUAPLUS_INLINE int LuaState::VYield(int nresults, void* ctx) {
	return lua_vyield(LuaState_to_lua_State(this), nresults, ctx);
}


LUAPLUS_INLINE int LuaState::IYield(int nresults, int ctx) {
	return lua_vyield(LuaState_to_lua_State(this), nresults, ctx);
}

#endif /* LUA_EXT_RESUMABLEVM */


LUAPLUS_INLINE int LuaState::Yield_(int nresults) {
	return lua_yield(LuaState_to_lua_State(this), nresults);
}


LUAPLUS_INLINE int LuaState::Resume(int narg) {
	return lua_resume(LuaState_to_lua_State(this), narg);
}


LUAPLUS_INLINE int LuaState::Status() {
	return lua_status(LuaState_to_lua_State(this));
}


LUAPLUS_INLINE int LuaState::CoYield(int nresults)
{
	return lua_yield(LuaState_to_lua_State(this), nresults);
}

LUAPLUS_INLINE int LuaState::CoResume(int narg)
{
	return lua_resume(LuaState_to_lua_State(this), narg);
}

LUAPLUS_INLINE int LuaState::CoStatus()
{
	return lua_status(LuaState_to_lua_State(this));
}


/*
** garbage-collection function and options
*/
LUAPLUS_INLINE int LuaState::GC(int what, int data)
{
	return lua_gc(LuaState_to_lua_State(this), what, data); 
}


/*
** miscellaneous functions
*/
LUAPLUS_INLINE int LuaState::Error()
{
	return lua_error(LuaState_to_lua_State(this));
}


LUAPLUS_INLINE int LuaState::Next(int index)
{
	return lua_next(LuaState_to_lua_State(this), index);
}

LUAPLUS_INLINE void LuaState::Concat(int n)
{
	lua_concat(LuaState_to_lua_State(this), n);
}


LUAPLUS_INLINE lua_Alloc LuaState::GetAllocF(void **ud)
{
	return lua_getallocf(LuaState_to_lua_State(this), ud);
}


LUAPLUS_INLINE void LuaState::SetAllocF(lua_Alloc f, void *ud)
{
	lua_setallocf(LuaState_to_lua_State(this), f, ud);
}

// Helper functions
LUAPLUS_INLINE void LuaState::Pop()
{
	lua_pop(LuaState_to_lua_State(this), 1);
}

LUAPLUS_INLINE void LuaState::Pop(int amount)
{
	lua_pop(LuaState_to_lua_State(this), amount);
}

LUAPLUS_INLINE void LuaState::NewTable() {
	lua_newtable(LuaState_to_lua_State(this));
}

LUAPLUS_INLINE void LuaState::Register(const char* key, lua_CFunction f) {
	lua_register(LuaState_to_lua_State(this), key, f);
}

LUAPLUS_INLINE size_t LuaState::StrLen(int index) {
	return lua_strlen(LuaState_to_lua_State(this), index);
}

LUAPLUS_INLINE void LuaState::SetGlobal(const char* key) {
	return lua_setglobal(LuaState_to_lua_State(this), key);
}

// Debug functions.
LUAPLUS_INLINE int LuaState::GetStack(int level, lua_Debug* ar)
{
	return lua_getstack(LuaState_to_lua_State(this), level, ar);
}

LUAPLUS_INLINE int LuaState::GetInfo(const char* what, lua_Debug* ar)
{
	return lua_getinfo(LuaState_to_lua_State(this), what, ar);
}

LUAPLUS_INLINE const char* LuaState::GetLocal(const lua_Debug* ar, int n)
{
	return lua_getlocal(LuaState_to_lua_State(this), ar, n);
}

LUAPLUS_INLINE const char* LuaState::SetLocal(const lua_Debug* ar, int n)
{
	return lua_setlocal(LuaState_to_lua_State(this), ar, n);
}

LUAPLUS_INLINE const char* LuaState::GetUpValue(int funcindex, int n) {
	return lua_getupvalue(LuaState_to_lua_State(this), funcindex, n);
}

LUAPLUS_INLINE const char* LuaState::SetUpValue(int funcindex, int n) {
	return lua_setupvalue(LuaState_to_lua_State(this), funcindex, n);
}

LUAPLUS_INLINE int LuaState::SetHook(lua_Hook func, int mask, int count)
{
	return lua_sethook(LuaState_to_lua_State(this), func, mask, count);
}

LUAPLUS_INLINE lua_Hook LuaState::GetHook()
{
	return lua_gethook(LuaState_to_lua_State(this));
}

LUAPLUS_INLINE int LuaState::GetHookMask()
{
	return lua_gethookmask(LuaState_to_lua_State(this));
}

LUAPLUS_INLINE int LuaState::GetHookCount()
{
	return lua_gethookcount(LuaState_to_lua_State(this));
}


// fastref support
#if LUA_FASTREF_SUPPORT

LUAPLUS_INLINE int LuaState::FastRef() {
	return lua_fastref(LuaState_to_lua_State(this));
}


LUAPLUS_INLINE int LuaState::FastRefIndex(int index) {
	return lua_fastrefindex(LuaState_to_lua_State(this), index);
}


LUAPLUS_INLINE void LuaState::FastUnref(int ref) {
	return lua_fastunref(LuaState_to_lua_State(this), ref);
}


LUAPLUS_INLINE void LuaState::GetFastRef(int ref) {
	return lua_getfastref(LuaState_to_lua_State(this), ref);
}

#endif /* LUA_FASTREF_SUPPORT */


// lauxlib functions.
LUAPLUS_INLINE void LuaState::OpenLib(const char *libname, const luaL_Reg *l, int nup) {
	luaI_openlib(LuaState_to_lua_State(this), libname, l, nup);
}


LUAPLUS_INLINE void LuaState::LRegister(const char *libname, const luaL_Reg *l) {
	luaL_register(LuaState_to_lua_State(this), libname, l);
}


LUAPLUS_INLINE int LuaState::GetMetaField(int obj, const char *e) {
	return luaL_getmetafield(LuaState_to_lua_State(this), obj, e);
}


LUAPLUS_INLINE int LuaState::CallMeta(int obj, const char *e) {
	return luaL_callmeta(LuaState_to_lua_State(this), obj, e);
}


LUAPLUS_INLINE int LuaState::TypeError(int narg, const char* tname)
{
	return luaL_typerror(LuaState_to_lua_State(this), narg, tname);
}


LUAPLUS_INLINE int LuaState::ArgError(int narg, const char* extramsg)
{
	return luaL_argerror(LuaState_to_lua_State(this), narg, extramsg);
}


LUAPLUS_INLINE const char* LuaState::CheckLString(int numArg, size_t* len)
{
	return luaL_checklstring(LuaState_to_lua_State(this), numArg, len);
}


LUAPLUS_INLINE const char* LuaState::OptLString(int numArg, const char *def, size_t* len)
{
	return luaL_optlstring(LuaState_to_lua_State(this), numArg, def, len);
}


LUAPLUS_INLINE lua_Number LuaState::CheckNumber(int numArg)
{
	return luaL_checknumber(LuaState_to_lua_State(this), numArg);
}


LUAPLUS_INLINE lua_Number LuaState::OptNumber(int nArg, lua_Number def)
{
	return luaL_optnumber(LuaState_to_lua_State(this), nArg, def);
}


LUAPLUS_INLINE lua_Integer LuaState::CheckInteger(int numArg)
{
	return luaL_checkinteger(LuaState_to_lua_State(this), numArg);
}


LUAPLUS_INLINE lua_Integer LuaState::OptInteger(int nArg, lua_Integer def)
{
	return luaL_optinteger(LuaState_to_lua_State(this), nArg, def);
}


#if LUAPLUS_EXTENSIONS

LUAPLUS_INLINE lua_Integer LuaState::CheckBoolean(int numArg) {
	return luaL_checkboolean(LuaState_to_lua_State(this), numArg);
}


LUAPLUS_INLINE lua_Integer LuaState::OptBoolean(int nArg, lua_Integer def) {
	return luaL_optboolean(LuaState_to_lua_State(this), nArg, def);
}

#endif /* LUAPLUS_EXTENSIONS */


LUAPLUS_INLINE const char* LuaState::CheckString(int numArg)
{
	return luaL_checkstring(LuaState_to_lua_State(this), numArg);
}


LUAPLUS_INLINE const char* LuaState::OptString(int numArg, const char* def)
{
	return luaL_optlstring(LuaState_to_lua_State(this), numArg, def, NULL);
}


LUAPLUS_INLINE int LuaState::CheckInt(int numArg)
{
	return (int)luaL_checkint(LuaState_to_lua_State(this), numArg);
}


LUAPLUS_INLINE long LuaState::CheckLong(int numArg)
{
	return (long)luaL_checklong(LuaState_to_lua_State(this), numArg);
}


LUAPLUS_INLINE int LuaState::OptInt(int numArg, int def)
{
	return (int)luaL_optint(LuaState_to_lua_State(this), numArg, def);
}


LUAPLUS_INLINE long LuaState::OptLong(int numArg, int def)
{
	return (long)luaL_optlong(LuaState_to_lua_State(this), numArg, def);
}


LUAPLUS_INLINE void LuaState::CheckStack(int sz, const char* msg)
{
	luaL_checkstack(LuaState_to_lua_State(this), sz, msg);
}


LUAPLUS_INLINE void LuaState::CheckType(int narg, int t)
{
	luaL_checktype(LuaState_to_lua_State(this), narg, t);
}


LUAPLUS_INLINE void LuaState::CheckAny(int narg)
{
	luaL_checkany(LuaState_to_lua_State(this), narg);
}


LUAPLUS_INLINE LuaStackObject LuaState::NewMetaTable(const char* tname)
{
	luaL_newmetatable(LuaState_to_lua_State(this), tname);
	return LuaStackObject(this, GetTop());
}

	
LUAPLUS_INLINE void* LuaState::CheckUData(int ud, const char* tname)
{
	return luaL_checkudata(LuaState_to_lua_State(this), ud, tname);
}


LUAPLUS_INLINE int LuaState::Where(int lvl)
{
	luaL_where(LuaState_to_lua_State(this), lvl);
	return LuaStackObject(this, GetTop());
}

	
LUAPLUS_INLINE int LuaState::Ref(int t)
{
	return luaL_ref(LuaState_to_lua_State(this), t);
}


LUAPLUS_INLINE void LuaState::Unref(int t, int ref)
{
	luaL_unref(LuaState_to_lua_State(this), t, ref);
}


LUAPLUS_INLINE void LuaState::ArgCheck(bool condition, int numarg, const char* extramsg)
{
	luaL_argcheck(LuaState_to_lua_State(this), condition, numarg, extramsg);
}


LUAPLUS_INLINE const char* LuaState::GSub(const char *s, const char *p, const char *r)
{
	return luaL_gsub(LuaState_to_lua_State(this), s, p, r);
}


LUAPLUS_INLINE const char* LuaState::FindTable(int idx, const char *fname, int szhint)
{
	return luaL_findtable(LuaState_to_lua_State(this), idx, fname, szhint);
}


LUAPLUS_INLINE const char* LuaState::LTypeName(int index) {
	return luaL_typename(LuaState_to_lua_State(this), index);
}


LUAPLUS_INLINE void LuaState::GetMetaTable(const char* metatableName) {
	luaL_getmetatable(LuaState_to_lua_State(this), metatableName);
}


LUAPLUS_INLINE int LuaState::UpValueIndex(int i)
{
	return lua_upvalueindex(i);
}

LUAPLUS_INLINE int LuaState::LoadString(const char* str)
{
	return luaL_loadbuffer(LuaState_to_lua_State(this), str, strlen(str), str);
}

#if LUA_WIDESTRING

LUAPLUS_INLINE int LuaState::LoadWString(const lua_WChar* str)
{
	return luaL_loadwbuffer(LuaState_to_lua_State(this), str, lua_WChar_len(str), "name");
}

#endif /* LUA_WIDESTRING */

// Extra
LUAPLUS_INLINE LuaStackObject LuaState::BoxPointer(void* u)
{
	(*(void **)(lua_newuserdata(LuaState_to_lua_State(this), sizeof(void *))) = (u));  return LuaStackObject(*this, lua_gettop(LuaState_to_lua_State(this)));
}

LUAPLUS_INLINE void* LuaState::UnBoxPointer(int stackIndex)
{
	return (*(void **)(lua_touserdata(LuaState_to_lua_State(this), stackIndex)));
}




} // namespace LuaPlus

#endif // LUASTATE_INL
