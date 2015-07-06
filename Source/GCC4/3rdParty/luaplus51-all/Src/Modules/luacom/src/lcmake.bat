@echo off
set LUA_VERSION_NUMBER=%1
set DBG=NO
call tecmake vc6 %2 %3 %4 %5 %6 %7 %8 %9 "MF=luacom_lib"
call tecmake vc6 %2 %3 %4 %5 %6 %7 %8 %9 "MF=luacom_console"
call tecmake dll %2 %3 %4 %5 %6 %7 %8 %9 "MF=luacom_lib"
call tecmake dll %2 %3 %4 %5 %6 %7 %8 %9 "MF=luacom_console"

