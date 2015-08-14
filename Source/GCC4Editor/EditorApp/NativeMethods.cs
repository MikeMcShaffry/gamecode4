//========================================================================
// NativeMethods.cs - exposes the C++ DLL functions
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

using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;

//========================================================================
//
// This file sets up some of the groundwork enabling us to call functions
// in our unmanaged dll from C# code
//
//========================================================================

namespace EditorApp
{
    //
    // class NativeMethods                  - Chapter 22, page 767
    //
    // We also need to import some functions that will allow us to load
    // the dll, and free it when we're done.
    static class NativeMethods
    {
        const string editorDllName = "GCC4EditorDLL_2013.dll";

        [DllImport(editorDllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern int EditorMain(IntPtr instancePtrAddress, IntPtr hPrevInstancePtrAddress, IntPtr hWndPtrAddress, int nCmdShow, int screenWidth, int screenHeight);
        [DllImport(editorDllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void RenderFrame();
        [DllImport(editorDllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void Shutdown();
        [DllImport(editorDllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void WndProc(IntPtr hWndPtrAddress, int msg, int wParam, int lParam);

        [DllImport(editorDllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void SaveLevel(string lFileName);
        [DllImport(editorDllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void OpenLevel([MarshalAs(UnmanagedType.BStr)] string lFileName);

        [DllImport(editorDllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetNumActors();
        [DllImport(editorDllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void GetActorList(IntPtr actorIdArrayPtrAddress, int size);
        
        [DllImport(editorDllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern int PickActor(IntPtr hWndPtrAddress);
        [DllImport(editorDllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern int GetActorXmlSize(uint actorId);
        [DllImport(editorDllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void GetActorXml(IntPtr actorXMLPtrAddress, uint actorId);

        [DllImport(editorDllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int CreateActor([MarshalAs(UnmanagedType.BStr)] string lactorResource);
        [DllImport(editorDllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void ModifyActor([MarshalAs(UnmanagedType.BStr)] string lactorModXML);
        [DllImport(editorDllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern int DestroyActor(uint actorId);

        [DllImport(editorDllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern int GetLevelScriptAdditionsXmlSize();
        [DllImport(editorDllName, CallingConvention = CallingConvention.Cdecl)]
        public unsafe static extern void GetLevelScriptAdditionsXml(IntPtr actorXMLPtrAddress);    
    }
}
