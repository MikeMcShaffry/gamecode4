/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
** Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>
**
** This file is part of the Qt Script Generator project on Trolltech Labs.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#include "control.h"
#include "lexer.h"

Control::Control()
  : current_context(0),
    _M_skipFunctionBody(false),
    _M_lexer(0),
    _M_parser(0)
{
  pushContext();

  declareTypedef(findOrInsertName("__builtin_va_list",
		 strlen("__builtin_va_list")), 0);
}

Control::~Control()
{
  popContext();

  Q_ASSERT(current_context == 0);
}

Lexer *Control::changeLexer(Lexer *lexer)
{
  Lexer *old = _M_lexer;
  _M_lexer = lexer;
  return old;
}

Parser *Control::changeParser(Parser *parser)
{
  Parser *old = _M_parser;
  _M_parser = parser;
  return old;
}

Type *Control::lookupType(const NameSymbol *name) const
{
  Q_ASSERT(current_context != 0);

  return current_context->resolve(name);
}

void Control::declare(const NameSymbol *name, Type *type)
{
  //printf("*** Declare:");
  //printSymbol(name);
  //putchar('\n');
  Q_ASSERT(current_context != 0);

  current_context->bind(name, type);
}

void Control::pushContext()
{
  // printf("+Context\n");
  Context *new_context = new Context;
  new_context->parent = current_context;
  current_context = new_context;
}

void Control::popContext()
{
  // printf("-Context\n");
  Q_ASSERT(current_context != 0);

  Context *old_context = current_context;
  current_context = current_context->parent;

  delete old_context;
}

void Control::declareTypedef(const NameSymbol *name, Declarator *d)
{
  //  printf("declared typedef:");
  //  printSymbol(name);
  //  printf("\n");
  stl_typedef_table.insert(name, d);
}

bool Control::isTypedef(const NameSymbol *name) const
{
  //  printf("is typedef:");
  //  printSymbol(name);
  // printf("= %d\n", (stl_typedef_table.find(name) != stl_typedef_table.end()));

  return stl_typedef_table.contains(name);
}

QList<Control::ErrorMessage> Control::errorMessages () const
{
  return _M_error_messages;
}

void Control::clearErrorMessages ()
{
  _M_error_messages.clear ();
}

void Control::reportError (const ErrorMessage &errmsg)
{
    _M_error_messages.append(errmsg);
}

// kate: space-indent on; indent-width 2; replace-tabs on;
