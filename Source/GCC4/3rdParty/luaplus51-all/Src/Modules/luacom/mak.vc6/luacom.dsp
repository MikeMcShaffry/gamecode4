# Microsoft Developer Studio Project File - Name="luacom" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=luacom - Win32 Debug Lua4
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "luacom.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "luacom.mak" CFG="luacom - Win32 Debug Lua4"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "luacom - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "luacom - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "luacom - Win32 Release Lua4" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "luacom - Win32 Debug Lua4" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "luacom - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LUACOM_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /I "c:\Lua5.1\include" /I "..\include" /I "..\src\library" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LUACOM_EXPORTS" /D "LUA51" /D LUACOM_DLL=\"luacom51.dll\" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 lua5.1.lib shlwapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:"..\bin\luacom51.pdb" /debug /machine:I386 /out:"..\bin\luacom51.dll" /implib:"..\lib\luacom51.lib" /libpath:"c:\Lua5.1\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "luacom - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LUACOM_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "c:\Lua5.1\include" /I "..\include" /I "..\src\library" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LUACOM_EXPORTS" /D "LUA51" /D LUACOM_DLL=\"luacom51_d.dll\" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 lua5.1_d.lib shlwapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:"..\bin\luacom51_d.pdb" /debug /machine:I386 /out:"..\bin\luacom51_d.dll" /implib:"..\lib\luacom51_d.lib" /pdbtype:sept /libpath:"c:\Lua5.1\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "luacom - Win32 Release Lua4"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "luacom___Win32_Release_Lua4"
# PROP BASE Intermediate_Dir "luacom___Win32_Release_Lua4"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "luacom___Win32_Release_Lua4"
# PROP Intermediate_Dir "luacom___Win32_Release_Lua4"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /Zi /O2 /I "..\..\Lua5.1\include" /I "..\include" /I "..\src\library" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LUACOM_EXPORTS" /D "LUA51" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /I "..\..\Lua4\lua-4.0.1\include" /I "..\include" /I "..\src\library" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LUACOM_EXPORTS" /D "LUA4" /D LUACOM_DLL=\"luacom4.dll\" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 lua5.1.lib shlwapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:"..\bin\luacom.pdb" /debug /machine:I386 /out:"..\bin\luacom.dll" /implib:"..\lib\luacom.lib" /libpath:"..\..\Lua5.1\lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 lua4.lib lualib4.lib shlwapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:"..\bin\luacom4.pdb" /debug /machine:I386 /out:"..\bin\luacom4.dll" /implib:"..\lib\luacom4.lib" /libpath:"..\..\Lua4\lua-4.0.1\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "luacom - Win32 Debug Lua4"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "luacom___Win32_Debug_Lua4"
# PROP BASE Intermediate_Dir "luacom___Win32_Debug_Lua4"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "luacom___Win32_Debug_Lua4"
# PROP Intermediate_Dir "luacom___Win32_Debug_Lua4"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\..\Lua5.1\include" /I "..\include" /I "..\src\library" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LUACOM_EXPORTS" /D "LUA51" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\..\Lua4\lua-4.0.1\include" /I "..\include" /I "..\src\library" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LUACOM_EXPORTS" /D "LUA4" /D LUACOM_DLL=\"luacom4_d.dll\" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 lua5.1_d.lib shlwapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:"..\bin\luacom_d.pdb" /debug /machine:I386 /out:"..\bin\luacom_d.dll" /implib:"..\lib\luacom_d.lib" /pdbtype:sept /libpath:"..\..\Lua5.1\lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 lua4_d.lib lualib4_d.lib shlwapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:"..\bin\luacom4_d.pdb" /debug /machine:I386 /out:"..\bin\luacom4_d.dll" /implib:"..\lib\luacom4_d.lib" /pdbtype:sept /libpath:"..\..\Lua4\lua-4.0.1\lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "luacom - Win32 Release"
# Name "luacom - Win32 Debug"
# Name "luacom - Win32 Release Lua4"
# Name "luacom - Win32 Debug Lua4"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\library\LuaAux.cpp
# End Source File
# Begin Source File

SOURCE=..\src\library\luabeans.cpp
# End Source File
# Begin Source File

SOURCE=..\src\library\luacom.cpp
# End Source File
# Begin Source File

SOURCE=..\src\library\luacom5.lua

!IF  "$(CFG)" == "luacom - Win32 Release"

# Begin Custom Build
InputDir=\kepler-cvs\luacom\src\library
InputPath=..\src\library\luacom5.lua

"luacom5.loh" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	c:\lua5.1\bin2c5.1.exe $(InputPath) > $(InputDir)\luacom5.loh

# End Custom Build

!ELSEIF  "$(CFG)" == "luacom - Win32 Debug"

# Begin Custom Build
InputDir=\kepler-cvs\luacom\src\library
InputPath=..\src\library\luacom5.lua

"luacom5.loh" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	c:\lua5.1\bin2c5.1.exe $(InputPath) > $(InputDir)\luacom5.loh

# End Custom Build

!ELSEIF  "$(CFG)" == "luacom - Win32 Release Lua4"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "luacom - Win32 Debug Lua4"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\dll\luacom_dll.cpp
# End Source File
# Begin Source File

SOURCE=..\src\dll\luacom_dll.def

!IF  "$(CFG)" == "luacom - Win32 Release"

!ELSEIF  "$(CFG)" == "luacom - Win32 Debug"

!ELSEIF  "$(CFG)" == "luacom - Win32 Release Lua4"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "luacom - Win32 Debug Lua4"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\library\LuaCompat.c
# End Source File
# Begin Source File

SOURCE=..\src\library\tCOMUtil.cpp
# End Source File
# Begin Source File

SOURCE=..\src\library\tLuaCOM.cpp
# End Source File
# Begin Source File

SOURCE=..\src\library\tLuaCOMClassFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\src\library\tLuaCOMConnPoints.cpp
# End Source File
# Begin Source File

SOURCE=..\src\library\tLuaCOMEnumerator.cpp
# End Source File
# Begin Source File

SOURCE=..\src\library\tLuaCOMException.cpp
# End Source File
# Begin Source File

SOURCE=..\src\library\tLuaCOMTypeHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\src\library\tLuaControl.cpp
# End Source File
# Begin Source File

SOURCE=..\src\library\tLuaDispatch.cpp
# End Source File
# Begin Source File

SOURCE=..\src\library\tLuaObject.cpp
# End Source File
# Begin Source File

SOURCE=..\src\library\tLuaObjList.cpp
# End Source File
# Begin Source File

SOURCE=..\src\library\tLuaTLB.cpp
# End Source File
# Begin Source File

SOURCE=..\src\library\tLuaVector.cpp
# End Source File
# Begin Source File

SOURCE=..\src\library\tStringBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\library\tUtil.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\src\library\LuaAux.h
# End Source File
# Begin Source File

SOURCE=..\src\library\luabeans.h
# End Source File
# Begin Source File

SOURCE=..\include\luacom.h
# End Source File
# Begin Source File

SOURCE=..\src\dll\luacom_dll.h
# End Source File
# Begin Source File

SOURCE=..\src\library\luacom_internal.h
# End Source File
# Begin Source File

SOURCE=..\src\library\LuaCompat.h
# End Source File
# Begin Source File

SOURCE=..\src\library\tCOMUtil.h
# End Source File
# Begin Source File

SOURCE=..\src\library\tLuaCOM.h
# End Source File
# Begin Source File

SOURCE=..\src\library\tLuaCOMClassFactory.h
# End Source File
# Begin Source File

SOURCE=..\src\library\tLuaCOMConnPoints.h
# End Source File
# Begin Source File

SOURCE=..\src\library\tLuaCOMEnumerator.h
# End Source File
# Begin Source File

SOURCE=..\src\library\tLuaCOMException.h
# End Source File
# Begin Source File

SOURCE=..\src\library\tLuaCOMTypeHandler.h
# End Source File
# Begin Source File

SOURCE=..\src\library\tLuaControl.h
# End Source File
# Begin Source File

SOURCE=..\src\library\tLuaDispatch.h
# End Source File
# Begin Source File

SOURCE=..\src\library\tLuaObject.h
# End Source File
# Begin Source File

SOURCE=..\src\library\tLuaObjList.h
# End Source File
# Begin Source File

SOURCE=..\src\library\tLuaTLB.h
# End Source File
# Begin Source File

SOURCE=..\src\library\tLuaVector.h
# End Source File
# Begin Source File

SOURCE=..\src\library\tStringBuffer.h
# End Source File
# Begin Source File

SOURCE=..\src\library\tUtil.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
