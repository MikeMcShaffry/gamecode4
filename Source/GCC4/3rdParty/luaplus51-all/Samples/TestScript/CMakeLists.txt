PROJECT(TestScript)

ADD_SUBDIRECTORY(../../Src/LuaPlus LuaPlus)

INCLUDE_DIRECTORIES("../../Src")

ADD_EXECUTABLE(TestScript SimpleHeap.cpp TestScript.cpp)
TARGET_LINK_LIBRARIES(TestScript LuaPlus)

