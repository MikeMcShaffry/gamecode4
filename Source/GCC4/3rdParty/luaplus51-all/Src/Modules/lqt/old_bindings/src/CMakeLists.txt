#
# Copyright (c) 2007 Mauro Iazzi
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#

PROJECT(lqt)

INCLUDE(UsePkgConfig)
INCLUDE (${CMAKE_ROOT}/Modules/FindQt4.cmake)
INCLUDE (${QT_USE_FILE})

SET(LUA_PKG ${LUA_PKG} lua lua51 lua5.1 lua-5.1)
FOREACH(LN ${LUA_PKG})
	IF(NOT LUA_LINK_FLAGS)
		PKGCONFIG(${LN} LUA_INCLUDE_DIRS LUA_LIBRARY_DIRS LUA_LINK_FLAGS	LUA_CFLAGS)
	ENDIF(NOT LUA_LINK_FLAGS)
ENDFOREACH(LN)

QT_WRAP_CPP(fqt LuaFunction lqt_function.hpp)

ADD_LIBRARY(qt SHARED lqt_qt_utils.cpp lqt_common.cpp lqt_function.cpp ${LuaFunction})
TARGET_LINK_LIBRARIES(qt ${QT_LIBRARIES})
SET_SOURCE_FILES_PROPERTIES(lqt_qt_utils.cpp lqt_common.cpp lqt_function.cpp PROPERTIES COMPILE_FLAGS -ggdb)

FILE(GLOB BINDER_FILES lqt_bind_*.cpp)
FOREACH(FN ${BINDER_FILES})
STRING(REGEX REPLACE ".*lqt_bind_(.*).cpp" "\\1" CN ${FN})
#EXECUTE_PROCESS(COMMAND echo ${FN} ${CN})
ADD_LIBRARY(${CN} SHARED ${FN})
TARGET_LINK_LIBRARIES(${CN} ${QT_LIBRARIES})
SET_SOURCE_FILES_PROPERTIES(${FN} PROPERTIES COMPILE_FLAGS -ggdb)
ENDFOREACH(FN)


