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


#ifndef DECLARATOR_COMPILER_H
#define DECLARATOR_COMPILER_H

#include "default_visitor.h"
#include "codemodel.h"

#include <QtCore/QString>
#include <QtCore/QList>

class TokenStream;
class Binder;

class DeclaratorCompiler: protected DefaultVisitor
{
public:
  struct Parameter
  {
    TypeInfo type;
    QString name;
    QString defaultValueExpression;
    bool defaultValue;

    Parameter(): defaultValue(false) {}
  };

public:
  DeclaratorCompiler(Binder *binder);

  void run(DeclaratorAST *node);

  inline QString id() const { return _M_id; }
  inline QStringList arrayElements() const { return _M_array; }
  inline bool isFunction() const { return _M_function; }
  inline bool isVariadics() const { return _M_variadics; }
  inline bool isReference() const { return _M_reference; }
  inline int indirection() const { return _M_indirection; }
  inline QList<Parameter> parameters() const { return _M_parameters; }

protected:
  virtual void visitPtrOperator(PtrOperatorAST *node);
  virtual void visitParameterDeclaration(ParameterDeclarationAST *node);

private:
  Binder *_M_binder;
  TokenStream *_M_token_stream;

  bool _M_function;
  bool _M_reference;
  bool _M_variadics;
  int _M_indirection;
  QString _M_id;
  QStringList _M_array;
  QList<Parameter> _M_parameters;
};

#endif // DECLARATOR_COMPILER_H

// kate: space-indent on; indent-width 2; replace-tabs on;
