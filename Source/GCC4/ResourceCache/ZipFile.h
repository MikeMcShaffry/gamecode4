#pragma once
//========================================================================
// ZipFile.h : API to use Zip files
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

// --------------------------------------------------------------------------
// File:        ZipFile.h
//
// Purpose:     The declaration of a quick'n dirty ZIP file reader class.
//              Original code from Javier Arevalo.
//              Get zlib from http://www.cdrom.com/pub/infozip/zlib/
//
// class ZipFile - Chapter 8, page 214
// --------------------------------------------------------------------------

#include <stdio.h>

typedef std::map<std::string, int> ZipContentsMap;		// maps path to a zip content id

class ZipFile
{
  public:
    ZipFile() { m_nEntries=0; m_pFile=NULL; m_pDirData=NULL; }
    virtual ~ZipFile() { End(); fclose(m_pFile); }

    bool Init(const std::wstring &resFileName);
    void End();

    int GetNumFiles()const { return m_nEntries; }
    std::string GetFilename(int i) const;	
    int GetFileLen(int i) const;
    bool ReadFile(int i, void *pBuf);

	// Added to show multi-threaded decompression
	bool ReadLargeFile(int i, void *pBuf, void (*progressCallback)(int, bool &));

	int Find(const std::string &path) const;

	ZipContentsMap m_ZipContentsMap;

  private:
    struct TZipDirHeader;
    struct TZipDirFileHeader;
    struct TZipLocalHeader;

    FILE *m_pFile;		// Zip file
    char *m_pDirData;	// Raw data buffer.
    int  m_nEntries;	// Number of entries.

    // Pointers to the dir entries in pDirData.
    const TZipDirFileHeader **m_papDir;   
};


