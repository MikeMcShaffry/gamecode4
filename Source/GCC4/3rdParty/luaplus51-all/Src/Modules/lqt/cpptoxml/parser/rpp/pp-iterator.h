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

#ifndef PP_ITERATOR_H
#define PP_ITERATOR_H

#include <iterator>

namespace rpp {

class pp_null_output_iterator
  : public std::iterator<std::output_iterator_tag, void, void, void, void>
{
public:
  pp_null_output_iterator() {}

  template <typename _Tp>
  pp_null_output_iterator &operator=(_Tp const &)
  { return *this; }

  inline pp_null_output_iterator &operator * () { return *this; }
  inline pp_null_output_iterator &operator ++ () { return *this; }
  inline pp_null_output_iterator operator ++ (int) { return *this; }
};

template <typename _Container>
class pp_output_iterator
  : public std::iterator<std::output_iterator_tag, void, void, void, void>
{
  std::string &_M_result;

public:
  explicit pp_output_iterator(std::string &__result):
    _M_result (__result) {}

  inline pp_output_iterator &operator=(typename _Container::const_reference __v)
  {
    if (_M_result.capacity () == _M_result.size ())
      _M_result.reserve (_M_result.capacity () << 2);

    _M_result.push_back(__v);
    return *this;
  }

  inline pp_output_iterator &operator * () { return *this; }
  inline pp_output_iterator &operator ++ () { return *this; }
  inline pp_output_iterator operator ++ (int) { return *this; }
};

} // namespace rpp

#endif // PP_ITERATOR_H

// kate: space-indent on; indent-width 2; replace-tabs on;
