#pragma once
//========================================================================
// MemoryWatcher.h
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

#ifdef _DEBUG
#include "../Utilities/String.h"

#define GCC_MEMORY_WATCHER_DECLARATION() \
    public: \
        static long s_numAllocs; \
        static void* operator new(size_t size, int memBlock = _NORMAL_BLOCK, const char* filename = "<unknown>", int lineNum = 0); \
        static void operator delete(void* pPtr); \
        static void* operator new[](size_t size, int memBlock = _NORMAL_BLOCK, const char* filename = "<unknown>", int lineNum = 0); \
        static void operator delete[](void* pPtr); \
    private:

#define GCC_MEMORY_WATCHER_DEFINITION(_className_) \
    long _className_::s_numAllocs = 0; \
    void* _className_::operator new(size_t size, int memBlock, const char* filename, int lineNum) \
    { \
        void* pMem = malloc(size); \
        ++s_numAllocs; \
        GCC_LOG("MemoryWatcher", "NEW: 0x" + ToStr(reinterpret_cast<unsigned long>(pMem), 16) + " " + ToStr(size) + " (x" + ToStr(s_numAllocs) + ") -> " + std::string(filename) + ":" + ToStr(lineNum)); \
        return pMem; \
    } \
    void _className_::operator delete(void* pPtr) \
    { \
        GCC_LOG("MemoryWatcher", "DELETE: 0x" + ToStr(reinterpret_cast<unsigned long>(pPtr), 16) + " (x" + ToStr(s_numAllocs) + ")"); \
        free(pPtr); \
        --s_numAllocs; \
    } \
    void* _className_::operator new[](size_t size, int memBlock, const char* filename, int lineNum) \
    { \
        void* pMem = malloc(size); \
        GCC_LOG("MemoryWatcher", "NEW: 0x" + ToStr(reinterpret_cast<unsigned long>(pMem), 16) + " " + ToStr(size) + " (x" + ToStr(s_numAllocs) + ") -> " + std::string(filename) + ":" + ToStr(lineNum)); \
        return pMem; \
    } \
    void _className_::operator delete[](void* pPtr) \
    { \
        GCC_LOG("MemoryWatcher", "DELETE: 0x" + ToStr(reinterpret_cast<unsigned long>(pPtr), 16) + " (x" + ToStr(s_numAllocs) + ")"); \
        free(pPtr); \
        --s_numAllocs; \
    }

#else  // !defined(_DEBUG)
    #define GCC_MEMORY_WATCHER_DECLARATION() (sizeof(void*))
    #define GCC_MEMORY_WATCHER_DEFINITION(__className__) (sizeof(__className__))
#endif  // _DEBUG



