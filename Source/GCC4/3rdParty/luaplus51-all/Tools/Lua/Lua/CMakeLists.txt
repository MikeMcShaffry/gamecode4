INCLUDE(../../../LuaPlusBuildTypes.cmake)

INCLUDE_DIRECTORIES(../../../Src)

IF (MSVC)
SET(NETWORK_SRCS
		../../../Src/LuaRemoteDebuggingServer/LuaNetworkHelpers.cpp
		../../../Src/LuaRemoteDebuggingServer/LuaNetworkHelpers.h
		../../../Src/LuaRemoteDebuggingServer/LuaNetworkServer.cpp
		../../../Src/LuaRemoteDebuggingServer/LuaNetworkServer.h
		../../../Src/LuaRemoteDebuggingServer/LuaRemoteDebuggingServer.cpp
		../../../Src/LuaRemoteDebuggingServer/LuaRemoteDebuggingServer.h
)
ADD_DEFINITIONS(-DREMOTE_DEBUGGER)
ENDIF (MSVC)

SET(SRCS
		Lua.cpp
		${NETWORK_SRCS}
)

IF (MSVC)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
ENDIF (MSVC)

ADD_EXECUTABLE(LuaPlus ${SRCS})
TARGET_LINK_LIBRARIES(LuaPlus LuaPlusShared)
LUAPLUS_FIX_TARGET(LuaPlus)

INSTALL(TARGETS LuaPlus
	RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)