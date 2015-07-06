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


#include "class_compiler.h"
#include "lexer.h"
#include "binder.h"
#include "tokens.h"

ClassCompiler::ClassCompiler(Binder *binder)
  : _M_binder (binder),
     _M_token_stream(binder->tokenStream ()),
     name_cc(_M_binder),
     type_cc(_M_binder)
{
}

ClassCompiler::~ClassCompiler()
{
}

void ClassCompiler::run(ClassSpecifierAST *node)
{
  name_cc.run(node->name);
  _M_name = name_cc.name();
  _M_base_classes.clear();

  visit(node);
}

void ClassCompiler::visitClassSpecifier(ClassSpecifierAST *node)
{
  visit(node->base_clause);
}

void ClassCompiler::visitBaseSpecifier(BaseSpecifierAST *node)
{
  name_cc.run(node->name);
  QString name = name_cc.name();
  QString modifier = "";

  if (! name.isEmpty()) {
    _M_base_classes.append(name);
    switch (_M_token_stream->kind(node->access_specifier)) {
	    case Token_public:
		    modifier = modifier.prepend("public");
		    break;
	    case Token_private:
		    modifier = modifier.prepend("private");
		    break;
	    case Token_protected:
		    modifier = modifier.prepend("protected");
		    break;
	    default:
		    modifier = modifier.prepend("private");
		    break;
    }
    if (_M_token_stream->kind(node->virt)==Token_virtual) {
      modifier = modifier.prepend("virtual ");
    }
    _M_base_classes_mod.append(modifier);
  }
}


// kate: space-indent on; indent-width 2; replace-tabs on;
