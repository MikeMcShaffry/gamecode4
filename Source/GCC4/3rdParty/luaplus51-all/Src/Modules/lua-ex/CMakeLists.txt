include(../../../LuaPlusBuildTypes.cmake)

if (WIN32)
	set(SRCS
			w32api/dirent.c
			w32api/dirent.h
			w32api/ex.c
			w32api/pusherror.c
			w32api/pusherror.h
			w32api/spawn.c
			w32api/spawn.h
	)
else (WIN32)
	set(SRCS
			posix/ex.c
			posix/posix_spawn.c
			posix/posix_spawn.h
			spawn.c
			spawn.h
	)
endif (WIN32)

LUAPLUS_ADD_MODULE(lua-ex ${SRCS})
set_target_properties(lua-ex PROPERTIES OUTPUT_NAME ex)
