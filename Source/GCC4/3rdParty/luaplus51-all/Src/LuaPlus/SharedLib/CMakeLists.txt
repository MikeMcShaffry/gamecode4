INCLUDE(../../../LuaPlusBuildTypes.cmake)

SET(LUAPLUS_SRCS
		../auxiliar.c
		../auxiliar.h
		../LuaAutoBlock.h
		../LuaCall.cpp
		../LuaCall.h
		../LuaFunction.h
		../LuaHelper.h
		../LuaHelper_Object.h
		../LuaHelper_StackObject.h
		../LuaLink.h
		../LuaObject.cpp
		../LuaObject.h
		../LuaObject.inl
		../LuaPlus.cpp
		../LuaPlus.h
		../LuaPlusAddons.c
		../LuaPlusCD.h
		../LuaPlusFunctions.cpp
		../LuaPlusFunctions.h
		../LuaPlusInternal.h
		../LuaPlus_Libs.cpp
		../LuaStackObject.cpp
		../LuaStackObject.h
		../LuaStackObject.inl
		../LuaStackTableIterator.cpp
		../LuaStackTableIterator.h
		../LuaState.cpp
		../LuaState.h
		../LuaState.inl
		../LuaStateOutFile.cpp
		../LuaStateOutFile.h
		../LuaState_DumpObject.cpp
		../LuaTableIterator.cpp
		../LuaTableIterator.h
		../luathread.c
		../luathread.h
		../lwstrlib.c
		../pt.c
		../pt.h
		../srm.c
		../srm.h
		../src/lapi.c
		../src/lapi.h
		../src/lauxlib.c
		../src/lauxlib.h
		../src/lbaselib.c
		../src/lcode.c
		../src/lcode.h
		../src/ldblib.c
		../src/ldebug.c
		../src/ldebug.h
		../src/ldo.c
		../src/ldo.h
		../src/ldump.c
		../src/lfunc.c
		../src/lfunc.h
		../src/lgc.c
		../src/lgc.h
		../src/linit.c
		../src/liolib.c
		../src/llex.c
		../src/llex.h
		../src/llimits.h
		../src/lmathlib.c
		../src/lmem.c
		../src/lmem.h
		../src/loadlib.c
		../src/lobject.c
		../src/lobject.h
		../src/lopcodes.c
		../src/lopcodes.h
		../src/loslib.c
		../src/lparser.c
		../src/lparser.h
		../src/lstate.c
		../src/lstate.h
		../src/lstring.c
		../src/lstring.h
		../src/lstrlib.c
		../src/ltable.c
		../src/ltable.h
		../src/ltablib.c
		../src/ltm.c
		../src/ltm.h
		../src/lua.h
		../src/luaconf.h
		../src/lualib.h
		../src/lundump.c
		../src/lundump.h
		../src/lvm.c
		../src/lvm.h
		../src/lzio.c
		../src/lzio.h
		../src/print.c
	)

IF (WIN32)
	SET(LUAPLUS_SRCS ${LUAPLUS_SRCS} ../popen.c)
ENDIF (WIN32)

INCLUDE_DIRECTORIES(../src)

IF (MSVC)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
ENDIF (MSVC)

ADD_DEFINITIONS(-DLUA_OPNAMES)

SET_SOURCE_FILES_PROPERTIES(../LuaObject.inl ../LuaStackObject.inl ../LuaState.inl PROPERTIES HEADER_FILE_ONLY true)

ADD_LIBRARY(LuaPlusShared SHARED ${LUAPLUS_SRCS})
SET_TARGET_PROPERTIES(LuaPlusShared PROPERTIES COMPILE_FLAGS "-DLUAPLUS_BUILDDLL -DLUAPLUS_DLL")
SET_TARGET_PROPERTIES(LuaPlusShared PROPERTIES OUTPUT_NAME LuaPlus_1100)
SET_TARGET_PROPERTIES(LuaPlusShared PROPERTIES BUILD_WITH_INSTALL_RPATH 1 INSTALL_NAME_DIR @executable_path)

LUAPLUS_FIX_TARGET(LuaPlusShared)

#INSTALL(TARGETS LuaPlusShared
#	RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)