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


#include "codemodel_finder.h"
#include "codemodel.h"
#include "binder.h"

CodeModelFinder::CodeModelFinder(CodeModel *model, Binder *binder)
  : _M_model(model),
    _M_binder (binder),
    _M_token_stream(binder->tokenStream ()),
    name_cc(_M_binder),
    _M_resolve_policy(ResolveItem)
{
}

CodeModelFinder::~CodeModelFinder()
{
}

ScopeModelItem CodeModelFinder::resolveScope(NameAST *name, ScopeModelItem scope)
{
  Q_ASSERT(scope != 0);

  ResolvePolicy saved_resolve_policy = _M_resolve_policy;
  _M_resolve_policy = ResolveScope;

  ScopeModelItem old = changeCurrentScope(scope);

  visit(name);
  ScopeModelItem result = _M_current_scope;

  changeCurrentScope(old); // restore

  _M_resolve_policy = saved_resolve_policy;

  return result;
}

ScopeModelItem CodeModelFinder::changeCurrentScope(ScopeModelItem scope)
{
  ScopeModelItem old = _M_current_scope;
  _M_current_scope = scope;
  return old;
}

void CodeModelFinder::visitName(NameAST *node)
{
  visitNodes(this, node->qualified_names);

  if (_M_resolve_policy == ResolveItem)
    visit(node->unqualified_name);
}

void CodeModelFinder::visitUnqualifiedName(UnqualifiedNameAST *node)
{
  if (!_M_current_scope)
    {
      // nothing to do
      return;
    }

  name_cc.run(node);
  QString id = name_cc.name();

  if (ClassModelItem klass = _M_current_scope->findClass(id))
    {
      _M_current_scope = klass;
    }
  else if (NamespaceModelItem parentNamespace = model_safe_cast<NamespaceModelItem>(_M_current_scope))
    {
      NamespaceModelItem ns = parentNamespace->findNamespace(id);
      _M_current_scope = model_static_cast<ScopeModelItem>(ns);
    }
  else if (FileModelItem file = model_safe_cast<FileModelItem>(_M_current_scope))
    {
      NamespaceModelItem ns = file->findNamespace(id);
      _M_current_scope = model_static_cast<ScopeModelItem>(ns);
    }
  }

// kate: space-indent on; indent-width 2; replace-tabs on;

