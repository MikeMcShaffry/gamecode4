//========================================================================
// String.cpp : Defines some useful string utility functions
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
//
// String manipulation routines
//
// Not described in the book, but very useful for printing multiline text
// Used in ..\Win32\Fonts.cpp
//========================================================================

#include "GameCodeStd.h"
#include "string.h"

using std::string;

// Remove all leading whitespace
void TrimLeft(std::wstring &s)
{
	// find first non-space character
	int i = 0;
	int len = (int)s.length();
	while( i <  len )
	{
		TCHAR ch = s[i];			
		int white = 
		#ifdef UNICODE
				iswspace( ch );
		#else
				isspace( ch );
		#endif
		if (!white)
			break;
		++i;
	}

	if (i<len)
		s = s.substr(i);
}



/*****************
void RemoveFirstLine(std::wstring &src, std::wstring &result)
{
	int breakPosition = (int)src.find('\n');
	result = _T("");
	if(breakPosition != std::string.npos)	//if found...
	{
		int len = (int)src.length();
		result = src.substr(0, breakPosition);
		src = src.substr(breakPosition+1, (len-breakPosition)-1);		// skip the '/n'
	}
	else
	{
		result = src;
		src = _T("");
	}
}



int CountLines(const std::wstring &s)
{
	int lines = 0;
	int breakPos = 0;

	do 
	{
		++lines;
		breakPos = (int)s.find('\n', breakPos+1);
	} while (breakPos != std::string.npos);

	return lines;
}	
*********************/


// The following function was found on http://xoomer.virgilio.it/acantato/dev/wildcard/wildmatch.html, where it was attributed to 
// the C/C++ Users Journal, written by Mike Cornelison. It is a little ugly, but it is FAST. Use this as an excercise in not reinventing the
// wheel, even if you see gotos. 

BOOL WildcardMatch(const char *pat, const char *str) {
   int i, star;

new_segment:

   star = 0;
   if (*pat == '*') {
      star = 1;
      do { pat++; } while (*pat == '*'); /* enddo */
   } /* endif */

test_match:

   for (i = 0; pat[i] && (pat[i] != '*'); i++) {
      //if (mapCaseTable[str[i]] != mapCaseTable[pat[i]]) {
	  if (str[i] != pat[i]) {
         if (!str[i]) return 0;
         if ((pat[i] == '?') && (str[i] != '.')) continue;
         if (!star) return 0;
         str++;
         goto test_match;
      }
   }
   if (pat[i] == '*') {
      str += i;
      pat += i;
      goto new_segment;
   }
   if (!str[i]) return 1;
   if (i && pat[i - 1] == '*') return 1;
   if (!star) return 0;
   str++;
   goto test_match;
}


//-----------------------------------------------------------------------------
// Name: AnsiToWideCch()
// Desc: This is a UNICODE conversion utility to convert a CHAR string into a
//       WCHAR string. 
//       cchDestChar is the size in TCHARs of wstrDestination.  Be careful not to 
//       pass in sizeof(strDest) 
//-----------------------------------------------------------------------------
HRESULT AnsiToWideCch( WCHAR* wstrDestination, const CHAR* strSource, 
                                     int cchDestChar )
{
    if( wstrDestination==NULL || strSource==NULL || cchDestChar < 1 )
        return E_INVALIDARG;

    int nResult = MultiByteToWideChar( CP_ACP, 0, strSource, -1, 
                                       wstrDestination, cchDestChar );
    wstrDestination[cchDestChar-1] = 0;
    
    if( nResult == 0 )
        return E_FAIL;
    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: WideToAnsi()
// Desc: This is a UNICODE conversion utility to convert a WCHAR string into a
//       CHAR string. 
//       cchDestChar is the size in TCHARs of strDestination
//-----------------------------------------------------------------------------
HRESULT WideToAnsiCch( CHAR* strDestination, const WCHAR* wstrSource, 
                                     int cchDestChar )
{
    if( strDestination==NULL || wstrSource==NULL || cchDestChar < 1 )
        return E_INVALIDARG;

    int nResult = WideCharToMultiByte( CP_ACP, 0, wstrSource, -1, strDestination, 
                                       cchDestChar*sizeof(CHAR), NULL, NULL );
    strDestination[cchDestChar-1] = 0;
    
    if( nResult == 0 )
        return E_FAIL;
    return S_OK;
}





//-----------------------------------------------------------------------------
// Name: GenericToAnsi()
// Desc: This is a UNICODE conversion utility to convert a TCHAR string into a
//       CHAR string. 
//       cchDestChar is the size in TCHARs of strDestination
//-----------------------------------------------------------------------------
HRESULT GenericToAnsiCch( CHAR* strDestination, const TCHAR* tstrSource, 
                                           int cchDestChar )
{
    if( strDestination==NULL || tstrSource==NULL || cchDestChar < 1 )
        return E_INVALIDARG;

#ifdef _UNICODE
    return WideToAnsiCch( strDestination, tstrSource, cchDestChar );
#else
    strncpy( strDestination, tstrSource, cchDestChar );
    strDestination[cchDestChar-1] = '\0';
    return S_OK;
#endif   
}




//-----------------------------------------------------------------------------
// Name: GenericToWide()
// Desc: This is a UNICODE conversion utility to convert a TCHAR string into a
//       WCHAR string. 
//       cchDestChar is the size in TCHARs of wstrDestination.  Be careful not to 
//       pass in sizeof(strDest) 
//-----------------------------------------------------------------------------
HRESULT GenericToWideCch( WCHAR* wstrDestination, const TCHAR* tstrSource, 
                                           int cchDestChar )
{
    if( wstrDestination==NULL || tstrSource==NULL || cchDestChar < 1 )
        return E_INVALIDARG;

#ifdef _UNICODE
    wcsncpy( wstrDestination, tstrSource, cchDestChar );
    wstrDestination[cchDestChar-1] = L'\0';
    return S_OK;
#else
    return AnsiToWideCch( wstrDestination, tstrSource, cchDestChar );
#endif    
}



//-----------------------------------------------------------------------------
// Name: AnsiToGeneric()
// Desc: This is a UNICODE conversion utility to convert a CHAR string into a
//       TCHAR string. 
//       cchDestChar is the size in TCHARs of tstrDestination.  Be careful not to 
//       pass in sizeof(strDest) on UNICODE builds
//-----------------------------------------------------------------------------
HRESULT AnsiToGenericCch( TCHAR* tstrDestination, const CHAR* strSource, 
                                           int cchDestChar )
{
    if( tstrDestination==NULL || strSource==NULL || cchDestChar < 1 )
        return E_INVALIDARG;
        
#ifdef _UNICODE
    return AnsiToWideCch( tstrDestination, strSource, cchDestChar );
#else
    strncpy( tstrDestination, strSource, cchDestChar );
    tstrDestination[cchDestChar-1] = '\0';
    return S_OK;
#endif    
}





//-----------------------------------------------------------------------------
// Name: AnsiToGeneric()
// Desc: This is a UNICODE conversion utility to convert a WCHAR string into a
//       TCHAR string. 
//       cchDestChar is the size in TCHARs of tstrDestination.  Be careful not to 
//       pass in sizeof(strDest) on UNICODE builds
//-----------------------------------------------------------------------------
HRESULT WideToGenericCch( TCHAR* tstrDestination, const WCHAR* wstrSource, 
                                           int cchDestChar )
{
    if( tstrDestination==NULL || wstrSource==NULL || cchDestChar < 1 )
        return E_INVALIDARG;

#ifdef _UNICODE
    wcsncpy( tstrDestination, wstrSource, cchDestChar );
    tstrDestination[cchDestChar-1] = L'\0';    
    return S_OK;
#else
    return WideToAnsiCch( tstrDestination, wstrSource, cchDestChar );
#endif
}



std::string ws2s(const std::wstring& s)
{
    int slength = (int)s.length() + 1;
    int len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0)-1; 
    std::string r(len, '\0');
    WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, &r[0], len, 0, 0); 
    return r;
}


std::wstring s2ws(const std::string &s)
{
    int slength = (int)s.length() + 1;
	int len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0)-1;
    std::wstring r(len, '\0');
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, &r[0], len);
    return r;
}





string ToStr(int num, int base)
{
    char str[MAX_DIGITS_IN_INT];
    memset(str,0,MAX_DIGITS_IN_INT);
    _itoa_s(num,str,MAX_DIGITS_IN_INT,base);
    return (string(str));
}  // end ToStr()

string ToStr(unsigned int num, int base)
{
    char str[MAX_DIGITS_IN_INT];
    memset(str, 0, MAX_DIGITS_IN_INT);
    _ultoa_s((unsigned long)num, str, MAX_DIGITS_IN_INT, base);
    return (string(str));
}

string ToStr(unsigned long num, int base)
{
    char str[MAX_DIGITS_IN_INT];
    memset(str,0,MAX_DIGITS_IN_INT);
    _ultoa_s(num,str,MAX_DIGITS_IN_INT,base);
    return (string(str));
}

string ToStr(float num)
{
    char str[64];  // I'm sure this is overkill
    memset(str,0,64);
    _sprintf_p(str,64,"%f",num);
    return (string(str));
}

string ToStr(double num)
{
    char str[64];  // I'm sure this is overkill
    memset(str,0,64);
    _sprintf_p(str,64,"%fL",num);
    return (string(str));
}

string ToStr(bool val)
{
    return (string( (val == true ? "true" : "false") ));
}

string ToStr(const Vec3& vec)
{
    return string("(" + ToStr(vec.x) + "," + ToStr(vec.y) + "," + ToStr(vec.z) + ")");
}



//---------------------------------------------------------------------------------------------------------------------
// This is basically like the Perl split() function.  It splits str into substrings by cutting it at each delimiter.  
// The result is stored in vec.
//---------------------------------------------------------------------------------------------------------------------
void Split(const string& str, StringVec& vec, char delimiter)
{
	vec.clear();
	size_t strLen = str.size();
	if (strLen == 0)
		return;

	size_t startIndex = 0;
	size_t indexOfDel = str.find_first_of(delimiter, startIndex);
	while (indexOfDel != string::npos)
	{
		vec.push_back(str.substr(startIndex, indexOfDel-startIndex));
		startIndex=indexOfDel + 1;
		if (startIndex >= strLen)
			break;
		indexOfDel = str.find_first_of(delimiter, startIndex);
	}
	if(startIndex < strLen)
		vec.push_back(str.substr(startIndex));
}



void *
HashedString::hash_name( char const * pIdentStr )
{
	// Relatively simple hash of arbitrary text string into a
	// 32-bit identifier Output value is
	// input-valid-deterministic, but no guarantees are made
	// about the uniqueness of the output per-input
	//
	// Input value is treated as lower-case to cut down on false
	// separations cause by human mistypes. Sure, it could be
	// construed as a programming error to mix up your cases, and
	// it cuts down on permutations, but in Real World Usage
	// making this text case-sensitive will likely just lead to
	// Pain and Suffering.
	//
	// This code lossely based upon the adler32 checksum by Mark
	// Adler and published as part of the zlib compression
	// library sources.

	// largest prime smaller than 65536
	unsigned long BASE = 65521L;

	// NMAX is the largest n such that 255n(n+1)/2 +
	// (n+1)(BASE-1) <= 2^32-1
	unsigned long NMAX = 5552;

#define DO1(buf,i)  {s1 += tolower(buf[i]); s2 += s1;}
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);

    if (pIdentStr == NULL)
		return NULL;

    unsigned long s1 = 0;
    unsigned long s2 = 0;

	for ( size_t len = strlen( pIdentStr ); len > 0 ; )
	{
        unsigned long k = len < NMAX ? len : NMAX;

		len -= k;

		while (k >= 16)
		{
            DO16(pIdentStr);
			pIdentStr += 16;
            k -= 16;
        }
		
        if (k != 0) do
		{
			s1 += tolower( *pIdentStr++ );
			s2 += s1;
        } while (--k);
		
        s1 %= BASE;
        s2 %= BASE;
    }

#pragma warning(push)
#pragma warning(disable : 4312)

    return reinterpret_cast<void *>( (s2 << 16) | s1 );

#pragma warning(pop)
#undef DO1
#undef DO2
#undef DO4
#undef DO8
#undef DO16
}
