// tStringBuffer.h: interface for the tStringBuffer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSTRINGBUFFER_H__35A7FE20_7CD4_11D4_B881_0000B45D7541__INCLUDED_)
#define AFX_TSTRINGBUFFER_H__35A7FE20_7CD4_11D4_B881_0000B45D7541__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class tStringBuffer  
{
public:
	const char * getBuffer(void);
	void copyToBuffer(char *source);
	void copyToBuffer(char *source, size_t length);
	tStringBuffer();
	virtual ~tStringBuffer();

protected:
	size_t size;
	char * buffer;
};

#endif // !defined(AFX_TSTRINGBUFFER_H__35A7FE20_7CD4_11D4_B881_0000B45D7541__INCLUDED_)
