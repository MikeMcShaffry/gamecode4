#pragma once
//========================================================================
// Minidump.h : This is a crash trapper - similar to a UNIX style core dump
//
// Part of the GameCode4 Application
//
// GameCode4 is the sample application that encapsulates much of the source code
// discussed in "Game Coding Complete - 4th Edition" by Mike McShaffry and David
// "Rez" Graham, published by Charles River Media. 
// ISBN-10: 1133776574 | ISBN-13: 978-1133776574
//
// If this source code has found it's way to you, and you think it has helped you
// in any way, do the authors a favor and buy a new copy of the book - there are 
// detailed explanations in it that compliment this code well. Buy a copy at Amazon.com
// by clicking here: 
//    http://www.amazon.com/gp/product/1133776574/ref=olp_product_details?ie=UTF8&me=&seller=
//
// There's a companion web site at http://www.mcshaffry.com/GameCode/
// 
// The source code is managed and maintained through Google Code: 
//    http://code.google.com/p/gamecode4/
//
// (c) Copyright 2012 Michael L. McShaffry and David Graham
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser GPL v3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See 
// http://www.gnu.org/licenses/lgpl-3.0.txt for more details.
//
// You should have received a copy of the GNU Lesser GPL v3
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//========================================================================

//========================================================================
// This is a simple class for handling minidumps.  Here’s an example of this class at work:
// 
// MiniDumper gMiniDumper;
// int main()
// {
//    *(int *)0 = 12;      // CRASH!!!!! 
//    return 0;
// }
// 
// Just declare a global singleton of the MiniDumper, and when an unhandled exception comes 
// along, your player will get asked if he or she wants to write out some diagnostic 
// information. The minidump file will appear in the same directory as your executable, ready 
// for debugging.
// 
// If you want to save the minidump file with a different name, inherit from the MiniDump class 
// and overload VSetDumpFileName. One thing you might consider doing is putting a timestamp in 
// the dump file name, so that one minidump file doesn’t overwrite another. If you’d like to 
// include your own data stream, overload VGetUserStreamArray(). 


#include <Tlhelp32.h>
#include "dbghelp.h"

#include <list>

class MiniDumper
{
protected:
	static MiniDumper *gpDumper;
	static LONG WINAPI Handler( struct _EXCEPTION_POINTERS *pExceptionInfo );

	_EXCEPTION_POINTERS *m_pExceptionInfo;
	time_t m_lTime;

	TCHAR m_szDumpPath[_MAX_PATH];
	TCHAR m_szAppPath[_MAX_PATH];
	TCHAR m_szAppBaseName[_MAX_PATH];
	LONG WriteMiniDump(_EXCEPTION_POINTERS *pExceptionInfo );
	BOOL m_bHeadless;

	virtual void VSetDumpFileName(void);
	virtual MINIDUMP_USER_STREAM_INFORMATION *VGetUserStreamArray() { return NULL; }
	virtual const TCHAR *VGetUserMessage() { return _T(""); }

public:
	MiniDumper(bool headless);
};

class ModuleEntry
{
protected:
	MODULEENTRY32 m_me32;
	TCHAR *m_pFileVersionInfo;
	TCHAR *m_pProductVersion;

	void ReadFileVersionInfo(void);
	void FindProductVersion(void);

public:
	ModuleEntry(const MODULEENTRY32 &me);
	const MODULEENTRY32 *Get() const { return &m_me32; }
	const TCHAR *GetProductVersion() const { return m_pProductVersion; }
	virtual ~ModuleEntry();
};

#define USER_DATA_BUFFER_SIZE (4096)

