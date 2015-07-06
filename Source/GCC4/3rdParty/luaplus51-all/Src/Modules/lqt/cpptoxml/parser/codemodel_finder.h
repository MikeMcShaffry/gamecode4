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


#ifndef CODEMODEL_FINDER_H
#define CODEMODEL_FINDER_H

#include "default_visitor.h"
#include "codemodel_fwd.h"
#include "name_compiler.h"

class TokenStream;
class Binder;

class CodeModelFinder: protected DefaultVisitor
{
  enum ResolvePolicy
  {
    ResolveScope,
    ResolveItem
  };

public:
  CodeModelFinder(CodeModel *model, Binder *binder);
  virtual ~CodeModelFinder();

  ScopeModelItem resolveScope(NameAST *name, ScopeModelItem scope);

  inline CodeModel *model() const { return _M_model; }

protected:
  virtual void visitName(NameAST *node);
  virtual void visitUnqualifiedName(UnqualifiedNameAST *node);

  ScopeModelItem changeCurrentScope(ScopeModelItem scope);

private:
  CodeModel *_M_model;
  Binder *_M_binder;
  TokenStream *_M_token_stream;
  NameCompiler name_cc;

  ScopeModelItem _M_current_scope;
  ResolvePolicy _M_resolve_policy;
};

#endif // CODEMODEL_FINDER_H

// kate: space-indent on; indent-width 2; replace-tabs on;
