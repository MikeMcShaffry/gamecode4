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


#include "compiler_utils.h"
#include "type_compiler.h"
#include "name_compiler.h"
#include "declarator_compiler.h"
#include "ast.h"
#include "binder.h"

TypeInfo CompilerUtils::typeDescription(TypeSpecifierAST *type_specifier, DeclaratorAST *declarator, Binder *binder)
{
  TypeCompiler type_cc (binder);
  DeclaratorCompiler decl_cc (binder);

  type_cc.run (type_specifier);
  decl_cc.run (declarator);

  TypeInfo typeInfo;
  typeInfo.setQualifiedName (type_cc.qualifiedName ());
  typeInfo.setConstant (type_cc.isConstant ());
  typeInfo.setVolatile (type_cc.isVolatile ());
  typeInfo.setReference (decl_cc.isReference ());
  typeInfo.setIndirections (decl_cc.indirection ());
  typeInfo.setArrayElements (decl_cc.arrayElements ());

  return typeInfo;
}

// kate: space-indent on; indent-width 2; replace-tabs on;
