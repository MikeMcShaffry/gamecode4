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


#ifndef VISITOR_H
#define VISITOR_H

#include "ast.h"

class Visitor
{
public:
  Visitor();
  virtual ~Visitor();

  virtual void visit(AST *node);

protected:
  virtual void visitAccessSpecifier(AccessSpecifierAST *) {}
  virtual void visitAsmDefinition(AsmDefinitionAST *) {}
  virtual void visitBaseClause(BaseClauseAST *) {}
  virtual void visitBaseSpecifier(BaseSpecifierAST *) {}
  virtual void visitBinaryExpression(BinaryExpressionAST *) {}
  virtual void visitCastExpression(CastExpressionAST *) {}
  virtual void visitClassMemberAccess(ClassMemberAccessAST *) {}
  virtual void visitClassSpecifier(ClassSpecifierAST *) {}
  virtual void visitCompoundStatement(CompoundStatementAST *) {}
  virtual void visitCondition(ConditionAST *) {}
  virtual void visitConditionalExpression(ConditionalExpressionAST *) {}
  virtual void visitCppCastExpression(CppCastExpressionAST *) {}
  virtual void visitCtorInitializer(CtorInitializerAST *) {}
  virtual void visitDeclarationStatement(DeclarationStatementAST *) {}
  virtual void visitDeclarator(DeclaratorAST *) {}
  virtual void visitDeleteExpression(DeleteExpressionAST *) {}
  virtual void visitDoStatement(DoStatementAST *) {}
  virtual void visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST *) {}
  virtual void visitEnumSpecifier(EnumSpecifierAST *) {}
  virtual void visitEnumerator(EnumeratorAST *) {}
  virtual void visitExceptionSpecification(ExceptionSpecificationAST *) {}
  virtual void visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST *) {}
  virtual void visitExpressionStatement(ExpressionStatementAST *) {}
  virtual void visitForStatement(ForStatementAST *) {}
  virtual void visitFunctionCall(FunctionCallAST *) {}
  virtual void visitFunctionDefinition(FunctionDefinitionAST *) {}
  virtual void visitIfStatement(IfStatementAST *) {}
  virtual void visitIncrDecrExpression(IncrDecrExpressionAST *) {}
  virtual void visitInitDeclarator(InitDeclaratorAST *) {}
  virtual void visitInitializer(InitializerAST *) {}
  virtual void visitInitializerClause(InitializerClauseAST *) {}
  virtual void visitLabeledStatement(LabeledStatementAST *) {}
  virtual void visitLinkageBody(LinkageBodyAST *) {}
  virtual void visitLinkageSpecification(LinkageSpecificationAST *) {}
  virtual void visitMemInitializer(MemInitializerAST *) {}
  virtual void visitName(NameAST *) {}
  virtual void visitNamespace(NamespaceAST *) {}
  virtual void visitNamespaceAliasDefinition(NamespaceAliasDefinitionAST *) {}
  virtual void visitNewDeclarator(NewDeclaratorAST *) {}
  virtual void visitNewExpression(NewExpressionAST *) {}
  virtual void visitNewInitializer(NewInitializerAST *) {}
  virtual void visitNewTypeId(NewTypeIdAST *) {}
  virtual void visitOperator(OperatorAST *) {}
  virtual void visitOperatorFunctionId(OperatorFunctionIdAST *) {}
  virtual void visitParameterDeclaration(ParameterDeclarationAST *) {}
  virtual void visitParameterDeclarationClause(ParameterDeclarationClauseAST *) {}
  virtual void visitPostfixExpression(PostfixExpressionAST *) {}
  virtual void visitPrimaryExpression(PrimaryExpressionAST *) {}
  virtual void visitPtrOperator(PtrOperatorAST *) {}
  virtual void visitPtrToMember(PtrToMemberAST *) {}
  virtual void visitReturnStatement(ReturnStatementAST *) {}
  virtual void visitSimpleDeclaration(SimpleDeclarationAST *) {}
  virtual void visitSimpleTypeSpecifier(SimpleTypeSpecifierAST *) {}
  virtual void visitSizeofExpression(SizeofExpressionAST *) {}
  virtual void visitStringLiteral(StringLiteralAST *) {}
  virtual void visitSubscriptExpression(SubscriptExpressionAST *) {}
  virtual void visitSwitchStatement(SwitchStatementAST *) {}
  virtual void visitTemplateArgument(TemplateArgumentAST *) {}
  virtual void visitTemplateDeclaration(TemplateDeclarationAST *) {}
  virtual void visitTemplateParameter(TemplateParameterAST *) {}
  virtual void visitThrowExpression(ThrowExpressionAST *) {}
  virtual void visitTranslationUnit(TranslationUnitAST *) {}
  virtual void visitTryBlockStatement(TryBlockStatementAST *) {}
  virtual void visitTypeId(TypeIdAST *) {}
  virtual void visitTypeIdentification(TypeIdentificationAST *) {}
  virtual void visitTypeParameter(TypeParameterAST *) {}
  virtual void visitTypedef(TypedefAST *) {}
  virtual void visitUnaryExpression(UnaryExpressionAST *) {}
  virtual void visitUnqualifiedName(UnqualifiedNameAST *) {}
  virtual void visitUsing(UsingAST *) {}
  virtual void visitUsingDirective(UsingDirectiveAST *) {}
  virtual void visitWhileStatement(WhileStatementAST *) {}
  virtual void visitWinDeclSpec(WinDeclSpecAST *) {}
  virtual void visitQProperty(QPropertyAST *) {}
  virtual void visitForwardDeclarationSpecifier(ForwardDeclarationSpecifierAST *) {}
  virtual void visitQEnums(QEnumsAST *) {}

private:
  typedef void (Visitor::*visitor_fun_ptr)(AST *);
  static visitor_fun_ptr _S_table[];
};

template <class _Tp>
  void visitNodes(Visitor *v, const ListNode<_Tp> *nodes)
  {
    if (!nodes)
      return;

    const ListNode<_Tp>
      *it = nodes->toFront(),
      *end = it;

    do
      {
        v->visit(it->element);
        it = it->next;
      }
    while (it != end);
  }

#endif // VISITOR_H

// kate: space-indent on; indent-width 2; replace-tabs on;
