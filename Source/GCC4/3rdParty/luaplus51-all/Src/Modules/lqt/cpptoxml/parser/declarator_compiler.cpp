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


#include "declarator_compiler.h"
#include "name_compiler.h"
#include "type_compiler.h"
#include "compiler_utils.h"
#include "lexer.h"
#include "binder.h"
#include "tokens.h"

#include <qdebug.h>

DeclaratorCompiler::DeclaratorCompiler(Binder *binder)
  : _M_binder (binder), _M_token_stream (binder->tokenStream ())
{
}

void DeclaratorCompiler::run(DeclaratorAST *node)
{
  _M_id.clear();
  _M_parameters.clear();
  _M_array.clear();
  _M_function = false;
  _M_reference = false;
  _M_variadics = false;
  _M_indirection = 0;

  if (node)
    {
      NameCompiler name_cc(_M_binder);

      DeclaratorAST *decl = node;
      while (decl && decl->sub_declarator)
        decl = decl->sub_declarator;

      Q_ASSERT (decl != 0);

      name_cc.run(decl->id);
      _M_id = name_cc.name();
      _M_function = (node->parameter_declaration_clause != 0);
      if (node->parameter_declaration_clause && node->parameter_declaration_clause->ellipsis)
        _M_variadics = true;

      visitNodes(this, node->ptr_ops);
      visit(node->parameter_declaration_clause);

      if (const ListNode<ExpressionAST*> *it = node->array_dimensions)
        {
          it->toFront();
          const ListNode<ExpressionAST*> *end = it;

          do
            {
              QString elt;
              if (ExpressionAST *expr = it->element)
                {
                  const Token &start_token = _M_token_stream->token((int) expr->start_token);
                  const Token &end_token = _M_token_stream->token((int) expr->end_token);

                  elt += QString::fromUtf8(&start_token.text[start_token.position],
                                           (int) (end_token.position - start_token.position)).trimmed();
                }

              _M_array.append (elt);

              it = it->next;
            }
          while (it != end);
        }
    }
}

void DeclaratorCompiler::visitPtrOperator(PtrOperatorAST *node)
{
  std::size_t op =  _M_token_stream->kind(node->op);

  switch (op)
    {
      case '&':
        _M_reference = true;
        break;
      case '*':
        ++_M_indirection;
        break;

      default:
        break;
    }

  if (node->mem_ptr)
    {
#if defined(__GNUC__)
#warning "ptr to mem -- not implemented"
#endif
    }
}

void DeclaratorCompiler::visitParameterDeclaration(ParameterDeclarationAST *node)
{
  Parameter p;

  TypeCompiler type_cc(_M_binder);
  DeclaratorCompiler decl_cc(_M_binder);

  decl_cc.run(node->declarator);

  p.name = decl_cc.id();
  p.type = CompilerUtils::typeDescription(node->type_specifier, node->declarator, _M_binder);
  if (node->expression != 0)
    {
      const Token &start = _M_token_stream->token((int) node->expression->start_token);
      const Token &end = _M_token_stream->token((int) node->expression->end_token);
      int length = (int) (end.position - start.position);

      p.defaultValueExpression = QString();
      QString source = QString::fromUtf8(&start.text[start.position], length).trimmed();
      QStringList list = source.split("\n");


      for (int i=0; i<list.size(); ++i) {
          if (!list.at(i).startsWith("#"))
              p.defaultValueExpression += list.at(i).trimmed();
      }

      p.defaultValue = p.defaultValueExpression.size() > 0;

    }

  _M_parameters.append(p);
}

// kate: space-indent on; indent-width 2; replace-tabs on;
