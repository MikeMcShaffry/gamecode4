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

#ifndef PP_SYMBOL_H
#define PP_SYMBOL_H

namespace rpp {

class pp_symbol
{
  static rxx_allocator<char> &allocator_instance ()
  {
    static rxx_allocator<char>__allocator;
    return __allocator;
  }

public:
  static int &N()
  {
    static int __N;
    return __N;
  }

  static pp_fast_string const *get (char const *__data, std::size_t __size)
  {
    ++N();
    char *data = allocator_instance ().allocate (__size + 1);
    memcpy(data, __data, __size);
    data[__size] = '\0';

    char *where = allocator_instance ().allocate (sizeof (pp_fast_string));
    return new (where) pp_fast_string (data, __size);
  }

  template <typename _InputIterator>
  static pp_fast_string const *get (_InputIterator __first, _InputIterator __last)
  {
    ++N();
    std::ptrdiff_t __size;
#if defined(__SUNPRO_CC)
    std::distance (__first, __last, __size);
#else
    __size = std::distance (__first, __last);
#endif
    assert (__size >= 0 && __size < 512);

    char *data = allocator_instance ().allocate (__size + 1);
    std::copy (__first, __last, data);
    data[__size] = '\0';

    char *where = allocator_instance ().allocate (sizeof (pp_fast_string));
    return new (where) pp_fast_string (data, __size);
  }

  static pp_fast_string const *get(std::string const &__s)
  { return get (__s.c_str (), __s.size ()); }
};

} // namespace rpp

#endif // PP_SYMBOL_H

// kate: space-indent on; indent-width 2; replace-tabs on;
