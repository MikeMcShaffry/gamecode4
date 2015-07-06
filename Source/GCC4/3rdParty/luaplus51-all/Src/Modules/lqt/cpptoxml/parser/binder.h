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


#ifndef BINDER_H
#define BINDER_H

#include "default_visitor.h"
#include "codemodel.h"
#include "type_compiler.h"
#include "name_compiler.h"
#include "declarator_compiler.h"

class TokenStream;
class LocationManager;
class Control;
struct NameSymbol;

class Binder: protected DefaultVisitor
{
public:
  Binder(CodeModel *__model, LocationManager &__location, Control *__control = 0);
  virtual ~Binder();

  inline TokenStream *tokenStream() const { return _M_token_stream; }
  inline CodeModel *model() const { return _M_model; }
  ScopeModelItem currentScope();

  FileModelItem run(AST *node);

// utils
  TypeInfo qualifyType(const TypeInfo &type, const QStringList &context) const;

protected:
  virtual void visitAccessSpecifier(AccessSpecifierAST *);
  virtual void visitClassSpecifier(ClassSpecifierAST *);
  virtual void visitEnumSpecifier(EnumSpecifierAST *);
  virtual void visitEnumerator(EnumeratorAST *);
  virtual void visitFunctionDefinition(FunctionDefinitionAST *);
  virtual void visitLinkageSpecification(LinkageSpecificationAST *);
  virtual void visitNamespace(NamespaceAST *);
  virtual void visitSimpleDeclaration(SimpleDeclarationAST *);
  virtual void visitTemplateDeclaration(TemplateDeclarationAST *);
  virtual void visitTypedef(TypedefAST *);
  virtual void visitUsing(UsingAST *);
  virtual void visitUsingDirective(UsingDirectiveAST *);
  virtual void visitQProperty(QPropertyAST *);
  virtual void visitForwardDeclarationSpecifier(ForwardDeclarationSpecifierAST *);
  virtual void visitQEnums(QEnumsAST *);

private:

  int decode_token(std::size_t index) const;
  const NameSymbol *decode_symbol(std::size_t index) const;
  CodeModel::AccessPolicy decode_access_policy(std::size_t index) const;
  CodeModel::ClassType decode_class_type(std::size_t index) const;

  CodeModel::FunctionType changeCurrentFunctionType(CodeModel::FunctionType functionType);
  CodeModel::AccessPolicy changeCurrentAccess(CodeModel::AccessPolicy accessPolicy);
  NamespaceModelItem changeCurrentNamespace(NamespaceModelItem item);
  ClassModelItem changeCurrentClass(ClassModelItem item);
  FunctionDefinitionModelItem changeCurrentFunction(FunctionDefinitionModelItem item);
  TemplateParameterList changeTemplateParameters(TemplateParameterList templateParameters);

  void declare_symbol(SimpleDeclarationAST *node, InitDeclaratorAST *init_declarator);

  void applyStorageSpecifiers(const ListNode<std::size_t> *storage_specifiers, MemberModelItem item);
  void applyFunctionSpecifiers(const ListNode<std::size_t> *it, FunctionModelItem item);

  void updateItemPosition(CodeModelItem item, AST *node);

private:
  CodeModel *_M_model;
  LocationManager &_M_location;
  TokenStream *_M_token_stream;
  Control *_M_control;

  CodeModel::FunctionType _M_current_function_type;
  CodeModel::AccessPolicy _M_current_access;
  FileModelItem _M_current_file;
  NamespaceModelItem _M_current_namespace;
  ClassModelItem _M_current_class;
  FunctionDefinitionModelItem _M_current_function;
  EnumModelItem _M_current_enum;
  QStringList _M_context;
  TemplateParameterList _M_current_template_parameters; // ### check me
  QHash<QString, QString> _M_qualified_types;
  QHash<QString, int> _M_anonymous_enums;

protected:
  TypeCompiler type_cc;
  NameCompiler name_cc;
  DeclaratorCompiler decl_cc;
};

#endif // BINDER_H

// kate: space-indent on; indent-width 2; replace-tabs on;
