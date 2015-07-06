include(../../../LuaPlusBuildTypes.cmake)

set(SRCS )

COPY_FILE(${CMAKE_CURRENT_SOURCE_DIR}/expand.lua ${EXECUTABLE_OUTPUT_PATH}/lua/expand.lua SRCS)

add_custom_target(expand DEPENDS ${SRCS})
