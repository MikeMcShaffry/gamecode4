// tUtil.h: interface for the tUtil class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TUTIL_H__0B3E0B68_981F_11D3_A906_0004ACE655F9__INCLUDED_)
#define AFX_TUTIL_H__0B3E0B68_981F_11D3_A906_0004ACE655F9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <windows.h>
#include <stdio.h>
#include "tStringBuffer.h"	// Added by ClassView

class tUtil  
{
public:
	static void ShowHelp(const char* filename, unsigned long context);
	static char * strdup(const char *string);
	static void log(const char *who, const char *what,...);
	static void log_verbose(const char *who, const char *what,...);
	static void CloseLogFile(void);
	static bool OpenLogFile(const char *name);
	static BSTR string2bstr(const char *string, size_t len = -1);
	static const char * bstr2string(BSTR bstr);
	static const char * bstr2string(BSTR bstr, size_t& computedSize);
	static const char *GetErrorMessage(DWORD errorcode);
	static bool IsValidString(LPCTSTR string);

	static tStringBuffer string_buffer;
  static FILE* log_file;
};

// algumas macros uteis

#define UNUSED(x) (void)(x)

#define SAFEFREE(x) {if(x){free(x); (x) = NULL;}}

#define SAFEDELETE(x) {if(x){delete x; (x) = NULL;}}

#define SAFEDELETEARR(x) {if(x){delete[] x; (x) = NULL;}}

#endif // !defined(AFX_TUTIL_H__0B3E0B68_981F_11D3_A906_0004ACE655F9__INCLUDED_)
