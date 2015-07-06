
project (LUA C)

set(LUA_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lua/src)

# the following two variables are defined for the use of packages 
# that wish to link or compile against lua
set (LUA_INCLUDE_DIRS "${LUA_SRC_DIR}")
set (LUA_LIBRARIES lua)


macro(add_prefix _list _prefix)
	set(_tmp)
	foreach(_current ${${_list}})
		set(_tmp ${_tmp} ${_prefix}/${_current})
		#message( ${_prefix}/${_current})
	endforeach()
	set(${_list} ${_tmp})
endmacro()


# define the lua core source files
set (LUA_CORE_SRCS lapi.c lcode.c ldebug.c ldo.c ldump.c 
	 lfunc.c lgc.c llex.c lmem.c lobject.c lopcodes.c
	 lparser.c lstate.c lstring.c ltable.c ltm.c  lundump.c
	 lvm.c lzio.c)
add_prefix(LUA_CORE_SRCS ${LUA_SRC_DIR}/)

# define the lua lib source files
set (LUA_LIB_SRCS lauxlib.c lbaselib.c ldblib.c liolib.c
	lmathlib.c loslib.c ltablib.c lstrlib.c loadlib.c linit.c)
add_prefix(LUA_LIB_SRCS ${LUA_SRC_DIR}/)

# create the library
if(WIN32)
	add_definitions(-DLUA_BUILD_AS_DLL)
	if(MSVC)
		add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
	endif()
else()
	add_definitions(-DLUA_USE_LINUX)
endif()
set(LIBRARY_TYPE SHARED)

set(CMAKE_C_FLAGS_DEBUG -DLUA_USE_APICHECK)
set(CMAKE_CXX_FLAGS_DEBUG -DLUA_USE_APICHECK)


add_library (lua ${LIBRARY_TYPE} ${LUA_LIB_SRCS} ${LUA_CORE_SRCS})

if(UNIX)
   # "debug" is not a library, just a keyword, but does not work uppercase... CMake magic
	target_link_libraries (lua readline debug pthread)
	if(NOT APPLE)
		target_link_libraries (lua dl m history ncurses)
	endif()
endif()

# create the lue executable and link it to the lib
add_executable (luaexec ${LUA_SRC_DIR}/lua.c)
target_link_libraries (luaexec lua)

# name the executable lua just like the library
set_target_properties(luaexec PROPERTIES OUTPUT_NAME lua)

# create the luac thing (I have no clue) and link it
#add_executable (luac ${LUA_SRC_DIR}/luac.c ${LUA_SRC_DIR}/print.c)
#target_link_libraries (luac lua)
