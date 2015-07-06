/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
** Copyright 2005 Roberto Raggi <roberto@kdevelop.org>
**
** This file is part of $PRODUCT$.
**
** $CPP_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PP_CCTYPE_H
#define PP_CCTYPE_H

#include <cctype>

namespace rpp {

inline bool pp_isalpha (int __ch)
{ return std::isalpha ((unsigned char) __ch) != 0; }

inline bool pp_isalnum (int __ch)
{ return std::isalnum ((unsigned char) __ch) != 0; }

inline bool pp_isdigit (int __ch)
{ return std::isdigit ((unsigned char) __ch) != 0; }

inline bool pp_isspace (int __ch)
{ return std::isspace ((unsigned char) __ch) != 0; }

} // namespace rpp

#endif // PP_CCTYPE_H

// kate: space-indent on; indent-width 2; replace-tabs on;
