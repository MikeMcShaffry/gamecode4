// tLuaCOMException.h: interface for the tLuaCOMException class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TLUACOMEXCEPTION_H__26509908_AFD8_11D4_B882_0000B45D7541__INCLUDED_)
#define AFX_TLUACOMEXCEPTION_H__26509908_AFD8_11D4_B882_0000B45D7541__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <windows.h>

class tLuaCOMException  
{
public:
	void getDebugMessage(void);
	const char * getMessage(void);
  enum Errors {INTERNAL_ERROR, PARAMETER_OUT_OF_RANGE,
    TYPECONV_ERROR, COM_ERROR, COM_EXCEPTION, UNSUPPORTED_FEATURE,
    WINDOWS_ERROR, LUACOM_ERROR, MALLOC_ERROR
  };

  static const char *GetErrorMessage(DWORD errorcode);

	tLuaCOMException(Errors p_code, char *p_file, int p_line, const char *usermessage = NULL);
	virtual ~tLuaCOMException();

  Errors code;
  char *file;
  int line;
  const char *usermessage;

protected:
  static char *messages[];
};

#define LUACOM_EXCEPTION(x) throw tLuaCOMException(tLuaCOMException::x, \
  __FILE__, __LINE__)

#define LUACOM_EXCEPTION_MSG(x,y) throw tLuaCOMException(tLuaCOMException::x, \
  __FILE__, __LINE__, y)


#define CHECK(x,y) ((x) ? (void) 0 : LUACOM_EXCEPTION(y))

#define TYPECONV_ERROR(x) throw tLuaCOMException(tLuaCOMException::TYPECONV_ERROR, \
  __FILE__, __LINE__, x)

#define COM_ERROR(x) throw tLuaCOMException(tLuaCOMException::COM_ERROR, \
__FILE__, __LINE__, ((x) == NULL ? "Unknown error" : (x)))

#define COM_EXCEPTION(x) throw tLuaCOMException(tLuaCOMException::COM_EXCEPTION, \
  __FILE__, __LINE__, x)

#define CHECKPARAM(x) ((x) ? (void) 0 : LUACOM_EXCEPTION(PARAMETER_OUT_OF_RANGE))

#define CHECKPARAM_MSG(x,y) ((x) ? (void) 0 : LUACOM_EXCEPTION_MSG( \
  PARAMETER_OUT_OF_RANGE, y))

#define CHECKPRECOND(x) ((x) ? (void) 0 : LUACOM_EXCEPTION(INTERNAL_ERROR))

#define CHECKPOSCOND(x) CHECKPRECOND(x)

#define CHECKFEATURE(x, y) ((x) ? (void) 0 : tLuaCOMException( \
        tLuaCOMException::UNSUPPORTED_FEATURE, __FILE__, __LINE__, y))

#define INTERNAL_ERROR() LUACOM_EXCEPTION(INTERNAL_ERROR)

#define WINCHECK(x) ((x) ? (void) 0 : LUACOM_EXCEPTION(WINDOWS_ERROR))

#define LUACOM_ERROR(x) (throw tLuaCOMException( \
        tLuaCOMException::LUACOM_ERROR, __FILE__, __LINE__, x))

#define CHK_LCOM_ERR(x, y) ((x) ? (void) 0 : LUACOM_ERROR(y))

#define CHK_COM_ERR(x, y) (((x) == S_OK) ? (void) 0 : COM_ERROR(y))

#define CHKMALLOC(x) ((x) ? (void) 0 : LUACOM_EXCEPTION(MALLOC_ERROR))

#define CHK_COM_CODE(hr) CHK_COM_ERR(hr, tLuaCOMException::GetErrorMessage(hr))




#endif // !defined(AFX_TLUACOMEXCEPTION_H__26509908_AFD8_11D4_B882_0000B45D7541__INCLUDED_)
