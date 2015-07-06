======================
  Build Instructions
======================

see also http://code.google.com/p/lqt/


Supported:
Qt 4.4 / 4.5

Compilers:
GCC 4.0, 4.2, 4.3, 4.4
MSVC 2008, 2009

OS:
Mac OSX
Linux
Windows


1. CMake


Create an out-of-source directory, for instance relative to the lqt 
sources the directory ../build, then with qmake in your PATH 
variable call in ../build

Linux:
build> cmake ../lqt
build> make

 
Windows to build with the Visual Studio:
build> cmake ..\lqt
build> lqt.sln


Example lua files are in lqt/test, LUA_CPATH is needed:

Linux:
build> export LUA_CPATH=$PWD/lib/lib?.so
build> ./bin/lua ../lqt/test/webview.lua

Windows (debug build):
build> set LUA_CPATH=%CD%\bin\Debug\?.dll
build> bin\Debug\lua.exe ..\lqt\test\t7.lua

Mac
build>export LUA_CPATH=$PWD/lib/lib?.dylib
build> ./bin/lua ../lqt/test/webview.lua


Without any option it also builds Lua which could
be suppressed by -DSYSTEM_LUA
(TODO is FindLua broken?)


