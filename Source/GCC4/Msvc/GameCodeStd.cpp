//========================================================================
// GameCodeStd.cpp : source file that includes just the standard includes
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



// GameCodeStd.pch will be the pre-compiled header
#include "GameCodeStd.h"


//
// Colors are discussed in Chapter 13, page 453
//
Color g_White( 1.0f, 1.0f, 1.0f, fOPAQUE );	
Color g_Black( 0.0f, 0.0f, 0.0f, fOPAQUE );
Color g_Cyan( 0.0f, 1.0f, 1.0f, fOPAQUE );	
Color g_Red( 1.0f, 0.0f, 0.0f, fOPAQUE );
Color g_Green( 0.0f, 1.0f, 0.0f, fOPAQUE );
Color g_Blue( 0.0f, 0.0f, 1.0f, fOPAQUE );
Color g_Yellow( 1.0f, 1.0f, 0.0f, fOPAQUE );	
Color g_Gray40( 0.4f, 0.4f, 0.4f, fOPAQUE );
Color g_Gray25( 0.25f, 0.25f, 0.25f, fOPAQUE );
Color g_Gray65( 0.65f, 0.65f, 0.65f, fOPAQUE );
Color g_Transparent (1.0f, 0.0f, 1.0f, fTRANSPARENT );

// Note! If you change these direction vectors around to suit yourself
// Make sure you change it so it retains a left handed
// coordinate system 
Vec3 g_Right(1.0f, 0.0f, 0.0f);
Vec3 g_Up(0.0f, 1.0f, 0.0f);
Vec3 g_Forward(0.0f, 0.0f, 1.0f);


Vec4 g_Up4(g_Up.x, g_Up.y, g_Up.z, 0.0f);
Vec4 g_Right4(g_Right.x, g_Right.y, g_Right.z, 0.0f);
Vec4 g_Forward4(g_Forward.x, g_Forward.y, g_Forward.z, 0.0f);

const float fOPAQUE = 1.0f;
const int iOPAQUE = 1; 
const float fTRANSPARENT = 0.0f;
const int iTRANSPARENT = 0;

const int MEGABYTE = 1024 * 1024;
const float SIXTY_HERTZ = 16.66f;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;


// [rez] Uncomment this #define to dump every call to new to the output window
//#define MEM_LOG
#if defined(_DEBUG) && defined(MEM_LOG)

void* operator new(size_t size, int memType, const char* filename, int lineNum)
{
    // We have to do this old-school since we're not allowed to dynamically allocate memory here.
    char buffer[2048];
    int index = 0;
    index += strlen(ultoa(size, buffer, 10));
    strcpy(buffer+index, " -> ");
    index += 4;
    strcpy(buffer+index, filename);
    index += strlen(filename);
    buffer[index] = ':';
    ++index;
    index += strlen(itoa(lineNum, buffer+index, 10));
    buffer[index] = '\n';
    ++index;
    buffer[index] = '\0';
    ++index;
    OutputDebugStringA(buffer);

    return _malloc_dbg(size, memType, filename, lineNum);
}

void operator delete(void* pMemory)
{
    _free_dbg(pMemory, 1);
}

void* operator new[](size_t size, int memType, const char* filename, int lineNum)
{
    // We have to do this old-school since we're not allowed to dynamically allocate memory here.
    char buffer[2048];
    int index = 0;
    index += strlen(ultoa(size, buffer, 10));
    strcpy(buffer+index, " -> ");
    index += 4;
    strcpy(buffer+index, filename);
    index += strlen(filename);
    buffer[index] = ':';
    ++index;
    index += strlen(itoa(lineNum, buffer+index, 10));
    buffer[index] = '\n';
    ++index;
    buffer[index] = '\0';
    ++index;
    OutputDebugStringA(buffer);

    return _malloc_dbg(size, 1, filename, lineNum);
}

void operator delete[](void* pMemory)
{
    _free_dbg(pMemory, 1);
}

#endif
