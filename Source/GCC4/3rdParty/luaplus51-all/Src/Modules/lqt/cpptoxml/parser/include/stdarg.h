/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of $PRODUCT$.
**
** $CPP_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#ifndef __STDARG
#define __STDARG

#if !defined(_VA_LIST) && !defined(__VA_LIST_DEFINED)
#define _VA_LIST
#define _VA_LIST_DEFINED

typedef char *__va_list;
#endif
static float __va_arg_tmp;
typedef __va_list va_list;

#define va_start(list, start)               ((void)0)
#define __va_arg(list, mode, n)             ((void)0)
#define _bigendian_va_arg(list, mode, n)    ((void)0)
#define _littleendian_va_arg(list, mode, n) ((void)0)
#define va_end(list)                        ((void)0)
#define va_arg(list, mode)                  ((void)0)

typedef void *__gnuc_va_list;

#endif
