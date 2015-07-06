#include "lua.h"
#include "lauxlib.h"
#include "undefdups.h"
#include "p4/clientapi.h"
#include "p4/strtable.h"
#include "p4/spec.h"
#include "p4/ident.h"
#include "p4result.h"
#include "specmgr.h"
#include "clientuserlua.h"
#include "p4clientapi.h"
#include "p4mergedata.h"
#include "p4mapmaker.h"
#include <new>

static Ident ident = {
	IdentMagic "P4Lua" " (0.9)", "08/12/10"
};
//static Ident ident = {
//	IdentMagic "P4Lua" "/" ID_OS "/" ID_REL "/" ID_PATCH  " (" ID_API " API)",
//	 ID_Y "/" ID_M "/" ID_D
//};

#define	M_TAGGED		0x01
#define	M_PARSE_FORMS		0x02
#define	IS_TAGGED(x)		(x & M_TAGGED )
#define	IS_PARSE_FORMS(x)	(x & M_PARSE_FORMS )

#define P4_CONNECTION_METATABLE "P4.Connection"
#define P4_MAP_METATABLE "P4.Map"
#define P4_MERGEDATA_METATABLE "P4.MergeData"



// =====================
// ==== P4MergeData ====
// =====================

static P4MergeData *p4_checkmergedata(lua_State *L, int index) {
	void *ud = luaL_checkudata(L, index, P4_MERGEDATA_METATABLE);
	luaL_argcheck(L, ud != NULL, index, "P4.MergeData expected");
	return (P4MergeData*)ud;
}


static int p4_mergedata_gc(lua_State* L) {
	P4MergeData* m = p4_checkmergedata(L, 1);
	m->~P4MergeData();
	return 0;
}

static int p4_mergedata_run_merge(lua_State* L) {
	P4MergeData* m = p4_checkmergedata(L, 1);
    return m->RunMergeTool();
}

static int p4_mergedata_index_your_name(lua_State* L) {
	P4MergeData* m = p4_checkmergedata(L, 1);
	return m->GetYourName();
}

static int p4_mergedata_index_their_name(lua_State* L) {
	P4MergeData* m = p4_checkmergedata(L, 1);
	return m->GetTheirName();
}

static int p4_mergedata_index_base_name(lua_State* L) {
	P4MergeData* m = p4_checkmergedata(L, 1);
	return m->GetBaseName();
}

static int p4_mergedata_index_your_path(lua_State* L) {
	P4MergeData* m = p4_checkmergedata(L, 1);
	return m->GetYourPath();
}

static int p4_mergedata_index_their_path(lua_State* L) {
	P4MergeData* m = p4_checkmergedata(L, 1);
	return m->GetTheirPath();
}

static int p4_mergedata_index_base_path(lua_State* L) {
	P4MergeData* m = p4_checkmergedata(L, 1);
	return m->GetBasePath();
}

static int p4_mergedata_index_result_path(lua_State* L) {
	P4MergeData* m = p4_checkmergedata(L, 1);
	return m->GetResultPath();
}

static int p4_mergedata_index_merge_hint(lua_State* L) {
	P4MergeData* m = p4_checkmergedata(L, 1);
	return m->GetMergeHint();
}

static int p4_mergedata_index(lua_State *L) {
	P4MergeData* m = p4_checkmergedata(L, 1);
	const char* key = luaL_checklstring(L, 2, NULL);
	lua_getfield(L, lua_upvalueindex(1), key);
	lua_CFunction function = lua_tocfunction(L, -1);
	if (!function)
	    luaL_argerror(L, 2, "improper key");
	function(L);
	return 1;
}


static const struct luaL_reg p4_mergedata_index_funcs[] = {
	{ "your_name",					p4_mergedata_index_your_name },
	{ "their_name",					p4_mergedata_index_their_name },
	{ "base_name",					p4_mergedata_index_base_name },
	{ "your_path",					p4_mergedata_index_your_path },
	{ "their_path",					p4_mergedata_index_their_path },
	{ "base_path",					p4_mergedata_index_base_path },
	{ "result_path",				p4_mergedata_index_result_path },
	{ "merge_hint",					p4_mergedata_index_merge_hint },
	{ NULL, NULL },
};


static const struct luaL_reg p4_mergedata_funcs[] = {
	{ "__gc",						p4_mergedata_gc },
	{ "run_merge",					p4_mergedata_run_merge },
	{ NULL, NULL },
};


static void p4_mergedata_create_metatable(lua_State *L) {
	luaL_newmetatable(L, P4_MERGEDATA_METATABLE);			// metatable
	luaL_register(L, NULL, p4_mergedata_funcs);
	lua_pushliteral(L, "__index");
	lua_newtable(L);
	luaL_register(L, NULL, p4_mergedata_index_funcs);
	lua_pushcclosure(L, p4_mergedata_index, 1);
	lua_settable(L, -3);
	lua_pop(L, 1);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static P4MapMaker *p4_checkmap(lua_State *L, int index) {
  void *ud = luaL_checkudata(L, index, P4_MAP_METATABLE);
  luaL_argcheck(L, ud != NULL, index, "P4.Map expected");
  return (P4MapMaker*)ud;
}


static int p4_map_gc(lua_State* L) {
	P4MapMaker* m = p4_checkmap(L, 1);
	m->~P4MapMaker();
	return 0;
}

static int p4_map_as_array(lua_State* L) {
	P4MapMaker* m = p4_checkmap(L, 1);
    m->ToA();
	return 1;
}


static int p4_map_clear(lua_State* L) {
	P4MapMaker* m = p4_checkmap(L, 1);
    m->Clear();
    return 0;
}

static int p4_map_count(lua_State* L) {
	P4MapMaker* m = p4_checkmap(L, 1);
	lua_pushinteger(L, m->Count());
	return 1;
}

static int p4_map_includes(lua_State* L) {
	P4MapMaker* m = p4_checkmap(L, 1);
	m->Translate(2, 1);
	if (lua_isstring(L, -1)) {
		lua_pushboolean(L, true);
		return 1;
	}
	lua_pop(L, 1);

	m->Translate(2, 0);
	if (lua_isstring(L, -1)) {
		lua_pushboolean(L, true);
		return 1;
	}
	lua_pop(L, 1);
	lua_pushboolean(L, false);
	return 1;
}

//
// Insert a mapping into a P4::Map object. Can be called with either
// one, or two arguments. If one, it's assumed to be a string containing
// either a half-map, or both halves of the mapping.
//
static int p4_map_insert(lua_State* L) {
	P4MapMaker* m = p4_checkmap(L, 1);
    StrBuf	t;

	int argc = lua_gettop(L);
    if( argc < 2 || argc > 3 )
		return luaL_error(L, "P4::Map#insert takes 1 or 2 arguments");

    if( argc == 2 )
    {
		// A mapping with only a left hand side.
		m->Insert( 2 );
		return 0;
    }

	if( argc == 3 )
	{
		m->Insert( 2, 3 );
	}
	return 0;
}


static int p4_map_inspect(lua_State* L) {
	P4MapMaker* m = p4_checkmap(L, 1);
    StrBuf		b;
    StrBuf		tb;

    tb.Alloc( 32 );
    sprintf( tb.Text(), "%p", (void*) m );
    tb.SetLength();

    b << "#<P4::Map:" << tb << "> ";

    m->Inspect( b );
	lua_pushlstring(L, b.Text(), b.Length());
	return 1;
}


static int p4_map_is_empty(lua_State* L) {
	P4MapMaker* m = p4_checkmap(L, 1);
    lua_pushboolean(L, m->Count() == 0);
	return 1;
}


static int p4_map_join(lua_State* L) {
	P4MapMaker* l = p4_checkmap(L, 1);
	P4MapMaker* r = p4_checkmap(L, 2);
	P4MapMaker* m = P4MapMaker::Join(L, l, r);
	luaL_getmetatable( L, P4_MAP_METATABLE );
	lua_setmetatable(L, -2);
	return 1;
}


static int p4_map_lhs(lua_State* L) {
	P4MapMaker* m = p4_checkmap(L, 1);
    m->Lhs();
	return 1;
}


static int p4_map_reverse(lua_State* L) {
	P4MapMaker* m = p4_checkmap(L, 1);
	P4MapMaker* m2 = (P4MapMaker*)lua_newuserdata(L, sizeof(P4MapMaker));
	::new(m2) P4MapMaker(*m);

	luaL_getmetatable( L, P4_MAP_METATABLE );
	lua_setmetatable(L, -2);

	m2->Reverse();
	return 1;
}


static int p4_map_rhs(lua_State* L) {
	P4MapMaker* m = p4_checkmap(L, 1);
    m->Rhs();
	return 1;
}


//
// P4::Map#translate( string, fwd=true )
//
static int p4_map_translate(lua_State* L) {
	P4MapMaker* m = p4_checkmap(L, 1);

	int argc = lua_gettop(L);
	int optboolean;

    if( argc < 2 || argc > 3 )
		luaL_error(L, "Invalid arguments to P4::Map#translate. "
			"Pass the string you wish to translate, and an optional "
			"boolean to indicate whether translation should be in "
			"the forward direction." );

	optboolean = 1;
	if (lua_type(L, 3) == LUA_TBOOLEAN)
		optboolean = lua_toboolean(L, 3);
	m->Translate(2, optboolean);
	return 1;
}

static int p4_map_new(lua_State* L) {
	P4MapMaker* m = (P4MapMaker*)lua_newuserdata(L, sizeof(P4MapMaker));
	::new(m) P4MapMaker(L);

	luaL_getmetatable( L, P4_MAP_METATABLE );
	lua_setmetatable(L, -2);

	if (lua_istable(L, 1))
	{
		lua_pushnil(L);  /* first key */
		while (lua_next(L, 1) != 0) {
			m->Insert(-1);
			lua_pop(L, 1);
		}
	}

	return 1;
}

static const struct luaL_reg p4_map_funcs[] = {
	{ "__gc",						p4_map_gc },
	{ "as_array",					p4_map_as_array },
	{ "clear",						p4_map_clear },
	{ "count",						p4_map_count },
	{ "inspect",					p4_map_inspect },
	{ "insert",						p4_map_insert },
	{ "is_empty",					p4_map_is_empty },
//	{ "join",						p4_map_join },
	{ "lhs",						p4_map_lhs },
	{ "reverse",					p4_map_reverse },
	{ "rhs",						p4_map_rhs },
	{ "translate",					p4_map_translate },
	{ "includes",					p4_map_includes },
	{ NULL, NULL },
};


static void p4_map_create_metatable(lua_State *L) {
	luaL_newmetatable(L, P4_MAP_METATABLE);			// metatable
	luaL_register(L, NULL, p4_map_funcs);
	lua_pushliteral( L, "__index" );
	lua_pushvalue( L, -2 );
	lua_settable( L, -3 );
	lua_pop(L, 1);
}



static P4ClientApi *p4_checkconnection(lua_State *L, int index) {
  void *ud = luaL_checkudata(L, index, P4_CONNECTION_METATABLE);
  luaL_argcheck(L, ud != NULL, index, "P4.P4 expected");
  return (P4ClientApi*)ud;
}


static int p4_connection_index_api_level(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	lua_pushinteger(L, p4->GetApiLevel());
	return 1;
}

static int p4_connection_index_charset(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	lua_pushstring(L, p4->GetCharset().Text());
	return 1;
}

static int p4_connection_index_client(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	lua_pushstring(L, p4->GetClient().Text());
	return 1;
}

static int p4_connection_index_cwd(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	lua_pushstring(L, p4->GetCwd().Text());
	return 1;
}

static int p4_connection_index_host(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	lua_pushstring(L, p4->GetHost().Text());
	return 1;
}

static int p4_connection_index_keepalive(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	return p4->GetKeepAlive();
}

static int p4_connection_index_language(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	lua_pushstring(L, p4->GetLanguage().Text());
	return 1;
}

static int p4_connection_index_os(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	lua_pushstring(L, p4->GetOs().Text());
	return 1;
}

static int p4_connection_index_p4config_file(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	lua_pushstring(L, p4->GetConfig().Text());
	return 1;
}

static int p4_connection_index_password(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	lua_pushstring(L, p4->GetPassword().Text());
	return 1;
}

static int p4_connection_index_port(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	lua_pushstring(L, p4->GetPort().Text());
	return 1;
}

static int p4_connection_index_prog(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	lua_pushstring(L, p4->GetProg().Text());
	return 1;
}

static int p4_connection_index_resolver(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	lua_rawgeti(L, LUA_REGISTRYINDEX, p4->GetResolver());
	return 1;
}

static int p4_connection_index_ticket_file(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	lua_pushstring(L, p4->GetTicketFile().Text());
	return 1;
}

static int p4_connection_index_user(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	lua_pushstring(L, p4->GetUser().Text());
	return 1;
}

static int p4_connection_index_version(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	lua_pushstring(L, p4->GetVersion().Text());
	return 1;
}

static int p4_connection_index_errors(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	lua_rawgeti(L, LUA_REGISTRYINDEX, p4->GetErrors());
	return 1;
}

static int p4_connection_index_output(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	lua_rawgeti(L, LUA_REGISTRYINDEX, p4->GetOutput());
	return 1;
}

static int p4_connection_index_warnings(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	lua_rawgeti(L, LUA_REGISTRYINDEX, p4->GetWarnings());
	return 1;
}

static int p4_connection_index_tagged(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	lua_pushboolean(L, p4->IsTagged());
	return 1;
}


static int p4_connection_index_exception_level(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	lua_pushnumber(L, p4->ExceptionLevel());
	return 1;
}


static const struct luaL_reg p4_connection_index_funcs[] = {
	{ "api_level",		p4_connection_index_api_level },
	{ "charset",		p4_connection_index_charset },
	{ "client",			p4_connection_index_client },
	{ "cwd",			p4_connection_index_cwd },
	{ "host",			p4_connection_index_host },
	{ "keepalive",		p4_connection_index_keepalive },
	{ "language",		p4_connection_index_language },
	{ "os",				p4_connection_index_os },
	{ "p4config_file",	p4_connection_index_p4config_file },
	{ "password",		p4_connection_index_password },
	{ "port",			p4_connection_index_port },
	{ "prog",			p4_connection_index_prog },
	{ "resolver",		p4_connection_index_resolver },
	{ "ticket_file",	p4_connection_index_ticket_file },
	{ "user",			p4_connection_index_user },
	{ "version",		p4_connection_index_version },

	{ "errors",			p4_connection_index_errors },
	{ "output",			p4_connection_index_output },
	{ "warnings",		p4_connection_index_warnings },
	{ "tagged",			p4_connection_index_tagged },
	{ "exception_level",p4_connection_index_exception_level },
	{ NULL, NULL },
};



static int p4_connection_newindex_api_level(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	p4->SetApiLevel(luaL_checkint(L, 3));
	return 0;
}

static int p4_connection_newindex_charset(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	p4->SetCharset(luaL_checklstring(L, 3, NULL));
	return 0;
}

static int p4_connection_newindex_client(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	p4->SetClient(luaL_checklstring(L, 3, NULL));
	return 0;
}

static int p4_connection_newindex_cwd(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	const char* cwd = luaL_checklstring(L, 3, NULL);
	p4->SetCwd(cwd);
	return 0;
}

static int p4_connection_newindex_host(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	p4->SetHost(luaL_checklstring(L, 3, NULL));
	return 0;
}

static int p4_connection_newindex_input(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	p4->SetInput(3);
	return 0;
}

static int p4_connection_newindex_keepalive(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	p4->SetKeepAlive(3);
	return 0;
}

static int p4_connection_newindex_language(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	p4->SetLanguage(luaL_checklstring(L, 3, NULL));
	return 0;
}

static int p4_connection_newindex_password(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	p4->SetPassword(luaL_checklstring(L, 3, NULL));
	return 0;
}

static int p4_connection_newindex_port(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	p4->SetPort(luaL_checklstring(L, 3, NULL));
	return 0;
}

static int p4_connection_newindex_user(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	p4->SetUser(luaL_checklstring(L, 3, NULL));
	return 0;
}

static int p4_connection_newindex_prog(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	p4->SetProg(luaL_checklstring(L, 3, NULL));
	return 0;
}

static int p4_connection_newindex_version(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	p4->SetVersion(luaL_checklstring(L, 3, NULL));
	return 0;
}

static int p4_connection_newindex_ticketfile(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	p4->SetTicketFile(luaL_checklstring(L, 3, NULL));
	return 0;
}


static int p4_connection_newindex_maxlocktime(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	p4->SetMaxLockTime((int)luaL_checknumber(L, 3));
	return 0;
}


static int p4_connection_newindex_maxresults(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	p4->SetMaxResults((int)luaL_checknumber(L, 3));
	return 0;
}


static int p4_connection_newindex_maxscanrows(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	p4->SetMaxScanRows((int)luaL_checknumber(L, 3));
	return 0;
}


static int p4_connection_newindex_resolver(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	p4->SetResolver(3);
	return 0;
}


lua_Integer checkboolean (lua_State *L, int narg) {
	lua_Integer d = lua_toboolean(L, narg);
	if (d == 0 && !lua_isboolean(L, narg))  /* avoid extra test when d is not 0 */
		luaL_typerror(L, narg, lua_typename(L, LUA_TBOOLEAN));
	return d;
}

static int p4_connection_newindex_tagged(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	p4->Tagged(checkboolean(L, 3));
	return 0;
}


static int p4_connection_newindex_exception_level(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	p4->ExceptionLevel((int)luaL_checknumber(L, 3));
	return 0;
}


static const struct luaL_reg p4_connection_newindex_funcs[] = {
	{ "api_level",		p4_connection_newindex_api_level },
	{ "charset",		p4_connection_newindex_charset },
	{ "client",			p4_connection_newindex_client },
	{ "cwd",			p4_connection_newindex_cwd },
	{ "host",			p4_connection_newindex_host },
	{ "input",			p4_connection_newindex_input },
	{ "keepalive",		p4_connection_newindex_keepalive },
	{ "language",		p4_connection_newindex_language },
	{ "password",		p4_connection_newindex_password },
	{ "port",			p4_connection_newindex_port },
	{ "prog",			p4_connection_newindex_prog },
	{ "resolver",		p4_connection_newindex_resolver },
	{ "user",			p4_connection_newindex_user },
	{ "version",		p4_connection_newindex_version },
	{ "ticketfile",		p4_connection_newindex_ticketfile },

	{ "maxlocktime",	p4_connection_newindex_maxlocktime },
	{ "maxresults",		p4_connection_newindex_maxresults },
	{ "maxscanrows",	p4_connection_newindex_maxscanrows },
	{ "tagged",			p4_connection_newindex_tagged },
	{ "exception_level",p4_connection_newindex_exception_level },
	{NULL, NULL},
};



static int p4_connection_gc(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection( L, 1 );
	p4->~P4ClientApi();
	return 0;
}

static int p4_connection_tostring(lua_State *L) {
	return 0;
}

static int p4_connection_index(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection( L, 1 );
	const char* key = luaL_checklstring( L, 2, NULL );
	lua_getfield( L, lua_upvalueindex( 1 ), key );
	if ( !lua_isnil( L, -1 ) )
		return 1;
	lua_getfield( L, lua_upvalueindex( 2 ), key );
	lua_CFunction function = lua_tocfunction( L, -1 );
	if ( function )
		return function( L );
	lua_getfield( L, lua_upvalueindex( 3 ), key );
	if ( !lua_isnil( L, -1 ) )
		return 1;
	lua_getfield( L, lua_upvalueindex( 3 ), "__getattr" );
	if ( lua_isfunction( L, -1 ) )
	{
		lua_pushvalue( L, 1 );
		lua_pushvalue( L, 2 );
		lua_call( L, 2, 1 );
		if ( !lua_isnil( L, -1 ) )
			return 1;
	}
    luaL_argerror(L, 2, "improper key");
	return 1;
}

static int p4_connection_newindex(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	const char* key = luaL_checklstring(L, 2, NULL);
	lua_getfield(L, lua_upvalueindex(1), key);
	lua_CFunction function = lua_tocfunction(L, -1);
	if (!function)
	    luaL_argerror(L, 2, "improper key");
	function(L);
	return 0;
}


//
// Session connect/disconnect
//
static int p4_connection_connect(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	return p4->Connect();
}


static int p4_connection_disconnect(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	return p4->Disconnect();
}


static int p4_connection_connected(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	return p4->Connected();
}


static int p4_connection_env(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection( L, 1 );
	if ( lua_gettop( L ) == 2 )
	{
		if ( lua_isstring( L, 2 ) )
		{
			lua_pushstring( L, p4->GetEnv( lua_tostring( L, 2 ) ) );
			return 1;
		}
	}
	else
	{
		lua_pushboolean( L, p4->SetEnv( luaL_checklstring( L, 2, NULL ), luaL_checklstring( L, 3, NULL ) ) );
		return 1;
	}

	return 0;
}


/*******************************************************************************
 * Running commands.  General purpose Run method and method for supplying
 * input to "p4 xxx -i" commands
 ******************************************************************************/

static int p4_connection_run( lua_State *L ) {
	P4ClientApi* p4 = p4_checkconnection( L, 1 );
	const char* cmd;
	int argc;
	char** argv;
	int argstart;
	if ( lua_istable( L, 2 ) ) {
		lua_rawgeti( L, -1, 1 );
		if ( !lua_isstring( L, -1 ) )
			luaL_argerror( L, 2, "table's first entry should be the command" );
		cmd = lua_tostring( L, -1 );
		argc = lua_objlen( L, -2 ) - 1;
		argstart = 2;

		int origargc = argc;
		int i;
		for ( i = argstart; i < argstart + argc; ++i ) {
			lua_rawgeti( L, -2, i );
			if ( lua_istable( L, -1 ) )
				argc += lua_objlen( L, -1 ) - 1;
			lua_pop( L, 1 );
		}

		argv = (char**)malloc( sizeof(char*) * argc );
		argc = 0;
		for ( i = argstart; i < argstart + origargc; ++i ) {
			lua_rawgeti( L, -2, i );
			if ( lua_isstring( L, -1 ) || lua_isnumber( L, -1 ) ) {
				unsigned int len = lua_objlen( L, -1 );
				argv[argc] = (char*)malloc( len + 1 );
				strncpy( argv[argc], lua_tostring( L, -1 ), len );
				argv[argc][len] = 0;
				argc++;
			} else if (lua_istable(L, -1)) {
				lua_pushnil(L);  /* first key */
				while (lua_next(L, -2) != 0) {
					if ( lua_isstring( L, -1 ) || lua_isnumber( L, -1 ) ) {
						/* 'key' is at index -2 and 'value' at index -1 */
						unsigned int len = lua_objlen( L, -1 );
						argv[argc] = (char*)malloc( len + 1 );
						strncpy(argv[argc], lua_tostring(L, -1), len);
						argv[argc][len] = 0;
						argc++;
					}
					lua_pop(L, 1);  /* removes 'value'; keeps 'key' for next iteration */
				}
			}
			lua_pop( L, 1 );
		}
	}
	else
	{
		cmd = luaL_checklstring( L, 2, NULL );
		argc = lua_gettop( L ) - 2;
		argstart = 3;

		int origargc = argc;
		int i;
		for ( i = argstart; i < argstart + origargc; ++i ) {
			if ( lua_isnil( L, i ) ) {
				origargc = i - argstart;
				break;
			}
			else if ( lua_istable( L, i ) ) {
				size_t len = lua_objlen( L, i );
				if ( len > 0 ) {
					lua_rawgeti( L, i, 1 );
					if ( lua_istable( L, -1 ) ) {
						/* this is the table we want.  Likely, the caller wrapped it in {...}. */
						len = lua_objlen( L, -1 );
					}
					lua_pop( L, 1 );
					argc += len;
				}
				--argc;
			}
		}

		argv = (char**)malloc( sizeof(char*) * argc );
		argc = 0;
		for ( i = argstart; i < argstart + origargc; ++i ) {
			if ( lua_isstring( L, i )  ||  lua_isnumber( L, i ) ) {
				unsigned int len = lua_objlen( L, i );
				argv[ argc ] = (char*)malloc( len + 1 );
				strncpy( argv[ argc ], lua_tostring( L, i ), len );
				argv[ argc ][ len ] = 0;
				argc++;
			} else if (lua_istable(L, i)) {
				int innerTable = 0;
				size_t j;
				size_t len = lua_objlen( L, i );
				if ( len > 0 ) {
					lua_rawgeti( L, i, 1 );
					if ( lua_istable( L, -1 ) ) {
						innerTable = 1;
						len = lua_objlen( L, -1 );
					} else {
						lua_pop( L, 1 );
						lua_pushvalue( L, i );
					}
				}

				for ( j = 1; j <= len; ++j ) {
					lua_rawgeti( L, -1, j );
					if ( lua_isstring( L, -1 )  ||  lua_isnumber( L, -1 ) ) {
						/* 'key' is at index -2 and 'value' at index -1 */
						unsigned int len = lua_objlen( L, -1 );
						argv[ argc ] = (char*)malloc( len + 1 );
						strncpy( argv[ argc ], lua_tostring( L, -1 ), len );
						argv[ argc ][ len ] = 0;
						++argc;
					}
					lua_pop( L, 1 );
				}

				if ( len > 0 ) {
					lua_pop( L, 1 );
				}
			}
		}
	}

	int ret = p4->Run( cmd, argc, argv, lua_istable( L, 2 ) ? 2 : -1 );

	for (int i = 0; i < argc; ++i)
		free(argv[i]);
	free(argv);

	return ret;
}


static int p4_connection_format_spec(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	luaL_argcheck( L, lua_type( L, 3 ) == LUA_TTABLE, 2, "table expected" );
	return p4->FormatSpec( luaL_checkstring( L, 2 ), 3 );
}


static int p4_connection_parse_spec(lua_State *L) {
	P4ClientApi* p4 = p4_checkconnection(L, 1);
	return p4->ParseSpec( luaL_checkstring( L, 2 ), luaL_checkstring( L, 3 ) );
}


static void ReadValue( StrBuf& string, const char* ptr )
{
	for ( ; *ptr; ++ptr ) {
		if ( *ptr == '%'  &&
			( ( ptr[1] >= '0'  &&  ptr[1] <= '9' )  ||  ( ptr[1] >= 'A'  &&  ptr[1] <= 'F' ) )  &&
			( ( ptr[2] >= '0'  &&  ptr[2] <= '9' )  ||  ( ptr[2] >= 'A'  &&  ptr[2] <= 'F' ) ) ) {
			unsigned char ch;
			if ( ptr[1] >= '0'  &&  ptr[1] <= '9' )
				ch = ( ptr[1] - '0' ) << 4;
			else
				ch = ( ( ptr[1] - 'A' ) + 10 ) << 4;
			if ( ptr[2] >= '0'  &&  ptr[2] <= '9' )
				ch |= ( ptr[2] - '0' );
			else
				ch |= ( ptr[2] - 'A' ) + 10;
			ptr += 2;
			string.Extend( (char)ch );
		} else
			string.Extend( *ptr );
	}
	string.Extend( 0 );
}


static void ReadConnectionSettings( lua_State* L, StrBuf& connection )
{
	char* buffer = new char[ connection.Length() + 1 ];
	strcpy( buffer, connection.Text() );
	lua_createtable( L, 3, 0 );

	char* start = buffer;
	char* ptr = buffer;
	int index = 1;
	while ( *ptr ) {
		if ( *ptr == ',' ) {
			lua_pushlstring( L, start, ptr - start );
			lua_rawseti( L, -2, index );
			++index;

			++ptr;
			if ( *ptr == ' ' )
				++ptr;
			start = ptr;
		}
		else
			++ptr;
	}
	lua_pushlstring( L, start, ptr - start );
	lua_rawseti( L, -2, index );

	delete[] buffer;
}


static int p4_retrieve_gui_connections( lua_State *L ) {
	StrBuf settingsFilename;
	settingsFilename << getenv( "HOMEDRIVE" );
	settingsFilename << getenv( "HOMEPATH" );
	settingsFilename << "/.p4qt/settings";

	FILE* file = fopen( settingsFilename.Text(), "rt" );
	if ( !file )
		return 0;

	lua_newtable( L );						// p4Connections
	int p4ConnectionsCount = 0;
	bool inRecentConnections = false;

	char line[ 4096 ];
	while ( fgets( line, sizeof( line ) - 1, file ) ) {
		char* end = line + strlen( line ) - 1;
		while ( *end == 13  ||  *end == 10 )
			*end-- = 0;
		if ( inRecentConnections ) {
			if ( line[0] != '\t' ) {
				inRecentConnections = false;
			} else {
				StrBuf connection;
				ReadValue( connection, line + 1 );
				ReadConnectionSettings( L, connection );
				lua_rawseti( L, -2, ++p4ConnectionsCount );
			}
		}
		if ( !inRecentConnections ) {
			if ( strcmp( line, "RecentConnections:" ) == 0  ||  strcmp( line, "RecentConnection:" ) == 0 )
				inRecentConnections = true;
			else if ( strncmp( line, "LastConnection=", 15 ) == 0  ||  strncmp( line, "PrevConnection=", 15 ) == 0 )
			{
				lua_pushliteral( L, "LastConnection" );
				StrBuf connection;
				ReadValue( connection, line + 15 );
				ReadConnectionSettings( L, connection );
				lua_rawset( L, -3 );
			}
			else if ( strncmp( line, "OpenWorkspaces=", 15 ) == 0 )
			{
				lua_pushliteral( L, "OpenWorkspaces" );
				StrBuf connection;
				ReadValue( connection, line + 15 );
				ReadConnectionSettings( L, connection );
				lua_rawset( L, -3 );
			}
		}
	}

	return 1;
}


static const struct luaL_reg p4_connection_funcs[] = {
	{ "connect",					p4_connection_connect },
	{ "connected",					p4_connection_connected },
	{ "disconnect",					p4_connection_disconnect },
	{ "env",						p4_connection_env },
	{ "run",						p4_connection_run },
	{ "format_spec",				p4_connection_format_spec },
	{ "parse_spec",					p4_connection_parse_spec },
	{ "retrieve_gui_connections",	p4_retrieve_gui_connections },
	{ NULL, NULL },
};


static int p4_connection_create_metatable(lua_State *L) {
	luaL_newmetatable(L, P4_CONNECTION_METATABLE);			// metatable
	lua_pushliteral(L, "__gc");								// metatable __gc
	lua_pushcfunction(L, p4_connection_gc);					// metatable __gc function
	lua_settable(L, -3);									// metatable
//	lua_pushliteral(L, "__tostring");						// metatable __tostring
//	lua_pushcfunction(L, p4_connection_tostring);			// metatable __tostring function
//	lua_settable(L, -3);									// metatable
	lua_pushliteral(L, "__index");							// metatable __index
	lua_newtable(L);										// metatable __index table
	luaL_register (L, NULL, p4_connection_funcs);
	lua_newtable(L);										// metatable __index table table
	luaL_register(L, NULL, p4_connection_index_funcs);
	lua_pushvalue( L, -4 );
	lua_pushcclosure( L, p4_connection_index, 3 );
	lua_settable(L, -3);
	lua_pushliteral(L, "__newindex");
	lua_newtable(L);
	luaL_register(L, NULL, p4_connection_newindex_funcs);
	lua_pushcclosure(L, p4_connection_newindex, 1);
	lua_settable(L, -3);

	lua_pop(L, 1);
	return 0;
}


static int l_create_connection(lua_State *L) {
	P4ClientApi* p4 = (P4ClientApi*)lua_newuserdata(L, sizeof(P4ClientApi));
	::new(p4) P4ClientApi(L);

	luaL_getmetatable(L, P4_CONNECTION_METATABLE);
	lua_setmetatable(L, -2);

	return 1;
}


static int p4_identify(lua_State *L) {
    StrBuf	s;
    ident.GetMessage( &s );
	lua_pushstring(L, s.Text());
	return 1;
}


static const struct luaL_reg p4lib[] = {
	{ "P4", l_create_connection },
	{ "identify", p4_identify },
	{ "Map", p4_map_new },
	{ "Map_join", p4_map_join },
	{NULL, NULL},
};


extern "C" int luaopen_p4_p4api (lua_State *L) {
	luaL_register(L, "P4", p4lib);
	p4_connection_create_metatable(L);
	p4_map_create_metatable(L);
	p4_mergedata_create_metatable(L);
	return 1;
}

