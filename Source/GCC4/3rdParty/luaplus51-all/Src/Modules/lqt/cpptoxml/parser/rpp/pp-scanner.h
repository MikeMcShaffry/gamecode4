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

#ifndef PP_SCANNER_H
#define PP_SCANNER_H

namespace rpp {

struct pp_skip_blanks
{
  int lines;

  template <typename _InputIterator>
  _InputIterator operator () (_InputIterator __first, _InputIterator __last)
  {
    lines = 0;

    for (; __first != __last; lines += (*__first != '\n' ? 0 : 1), ++__first)
      {
        if (*__first == '\\')
          {
            _InputIterator __begin = __first;
            ++__begin;

            if (__begin != __last && *__begin == '\n')
                ++__first;
            else
              break;
          }
        else if (*__first == '\n' || !pp_isspace (*__first))
          break;
      }

    return __first;
  }
};

struct pp_skip_whitespaces
{
  int lines;

  template <typename _InputIterator>
  _InputIterator operator () (_InputIterator __first, _InputIterator __last)
  {
    lines = 0;

    for (; __first != __last; lines += (*__first != '\n' ? 0 : 1), ++__first)
      {
        if (! pp_isspace (*__first))
          break;
      }

    return __first;
  }
};

struct pp_skip_comment_or_divop
{
  int lines;

  template <typename _InputIterator>
  _InputIterator operator () (_InputIterator __first, _InputIterator __last)
  {
    enum {
      MAYBE_BEGIN,
      BEGIN,
      MAYBE_END,
      END,
      IN_COMMENT,
      IN_CXX_COMMENT
    } state (MAYBE_BEGIN);

    lines = 0;

    for (; __first != __last; lines += (*__first != '\n' ? 0 : 1), ++__first)
      {
        switch (state)
          {
            default:
              assert (0);
              break;

            case MAYBE_BEGIN:
              if (*__first != '/')
                return __first;

              state = BEGIN;
              break;

            case BEGIN:
              if (*__first == '*')
                state = IN_COMMENT;
              else if (*__first == '/')
                state = IN_CXX_COMMENT;
              else
                return __first;
              break;

            case IN_COMMENT:
              if (*__first == '*')
                state = MAYBE_END;
              break;

            case IN_CXX_COMMENT:
              if (*__first == '\n')
                return __first;
              break;

            case MAYBE_END:
              if (*__first == '/')
                state = END;
              else if (*__first != '*')
                state = IN_COMMENT;
              break;

            case END:
              return __first;
          }
      }

    return __first;
  }
};

struct pp_skip_identifier
{
  int lines;

  template <typename _InputIterator>
  _InputIterator operator () (_InputIterator __first, _InputIterator __last)
  {
    lines = 0;

    for (; __first != __last; lines += (*__first != '\n' ? 0 : 1), ++__first)
      {
        if (! pp_isalnum (*__first) && *__first != '_')
          break;
      }

    return __first;
  }
};

struct pp_skip_number
{
  int lines;

  template <typename _InputIterator>
  _InputIterator operator () (_InputIterator __first, _InputIterator __last)
  {
    lines = 0;

    for (; __first != __last; lines += (*__first != '\n' ? 0 : 1), ++__first)
      {
        if (! pp_isalnum (*__first) && *__first != '.')
          break;
      }

    return __first;
  }
};

struct pp_skip_string_literal
{
  int lines;

  template <typename _InputIterator>
  _InputIterator operator () (_InputIterator __first, _InputIterator __last)
  {
    enum {
      BEGIN,
      IN_STRING,
      QUOTE,
      END
    } state (BEGIN);

    lines = 0;

    for (; __first != __last; lines += (*__first != '\n' ? 0 : 1), ++__first)
      {
        switch (state)
          {
            default:
              assert (0);
              break;

            case BEGIN:
              if (*__first != '\"')
                return __first;
              state = IN_STRING;
              break;

            case IN_STRING:
              assert (*__first != '\n');

              if (*__first == '\"')
                state = END;
              else if (*__first == '\\')
                state = QUOTE;
              break;

            case QUOTE:
              state = IN_STRING;
              break;

            case END:
              return __first;
          }
      }

    return __first;
  }
};

struct pp_skip_char_literal
{
  int lines;

  template <typename _InputIterator>
  _InputIterator operator () (_InputIterator __first, _InputIterator __last)
  {
    enum {
      BEGIN,
      IN_STRING,
      QUOTE,
      END
    } state (BEGIN);

    lines = 0;

    for (; state != END && __first != __last; lines += (*__first != '\n' ? 0 : 1), ++__first)
      {
        switch (state)
          {
            default:
              assert (0);
              break;

            case BEGIN:
              if (*__first != '\'')
                return __first;
              state = IN_STRING;
              break;

            case IN_STRING:
              assert (*__first != '\n');

              if (*__first == '\'')
                state = END;
              else if (*__first == '\\')
                state = QUOTE;
              break;

            case QUOTE:
              state = IN_STRING;
              break;
          }
      }

    return __first;
  }
};

struct pp_skip_argument
{
  pp_skip_identifier skip_number;
  pp_skip_identifier skip_identifier;
  pp_skip_string_literal skip_string_literal;
  pp_skip_char_literal skip_char_literal;
  pp_skip_comment_or_divop skip_comment_or_divop;
  int lines;

  template <typename _InputIterator>
  _InputIterator operator () (_InputIterator __first, _InputIterator __last)
  {
    int depth = 0;
    lines = 0;

    while (__first != __last)
      {
        if (!depth && (*__first == ')' || *__first == ','))
          break;
        else if (*__first == '(')
          ++depth, ++__first;
        else if (*__first == ')')
          --depth, ++__first;
        else if (*__first == '\"')
          {
            __first = skip_string_literal (__first, __last);
            lines += skip_string_literal.lines;
          }
        else if (*__first == '\'')
          {
            __first = skip_char_literal (__first, __last);
            lines += skip_char_literal.lines;
          }
        else if (*__first == '/')
          {
            __first = skip_comment_or_divop (__first, __last);
            lines += skip_comment_or_divop.lines;
          }
        else if (pp_isalpha (*__first) || *__first == '_')
          {
            __first = skip_identifier (__first, __last);
            lines += skip_identifier.lines;
          }
        else if (pp_isdigit (*__first))
          {
            __first = skip_number (__first, __last);
            lines += skip_number.lines;
          }
        else if (*__first == '\n')
          {
            ++__first;
            ++lines;
          }
        else
          ++__first;
      }

    return __first;
  }
};

} // namespace rpp

#endif // PP_SCANNER_H

// kate: space-indent on; indent-width 2; replace-tabs on;
