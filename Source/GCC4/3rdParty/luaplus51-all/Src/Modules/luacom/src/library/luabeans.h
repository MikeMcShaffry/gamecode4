#ifndef LUABEANS_H
#define LUABEANS_H

class LuaBeans
{
public:

  class Events
  {
  public:
    lua_CFunction gettable;
    lua_CFunction settable;
    lua_CFunction noindex;
    lua_CFunction call;
    lua_CFunction gc;

    Events()
    {
      gettable = settable = noindex = call = gc = NULL;
    }
  };

	 //lua_State* getLuaState(void);

   static void createBeans(
     lua_State *L,
     const char* module_name,
     const char* name);

   static void Clean(void);

   static void registerObjectEvents(lua_State* L, class Events& events);
   static void registerPointerEvents(lua_State* L, class Events& events);
   static void  push(lua_State* L, void* userdata);
   static void* check_tag(lua_State* L, int index);
   static void* from_lua(lua_State* L, int index);

protected:
   static char* tag_name;
   static char* udtag_name;
   static Events* pEvents;
   static const char* module_name;
};
#endif
