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


#ifndef CODEMODEL_FWD_H
#define CODEMODEL_FWD_H

#include <codemodel_pointer.h>
#include <QtCore/QList>

// forward declarations
class CodeModel;
class _ArgumentModelItem;
class _ClassModelItem;
class _CodeModelItem;
class _EnumModelItem;
class _EnumeratorModelItem;
class _FileModelItem;
class _FunctionDefinitionModelItem;
class _FunctionModelItem;
class _NamespaceModelItem;
class _ScopeModelItem;
class _TemplateParameterModelItem;
class _TypeAliasModelItem;
class _VariableModelItem;
class _MemberModelItem;
class TypeInfo;

typedef CodeModelPointer<_ArgumentModelItem> ArgumentModelItem;
typedef CodeModelPointer<_ClassModelItem> ClassModelItem;
typedef CodeModelPointer<_CodeModelItem> CodeModelItem;
typedef CodeModelPointer<_EnumModelItem> EnumModelItem;
typedef CodeModelPointer<_EnumeratorModelItem> EnumeratorModelItem;
typedef CodeModelPointer<_FileModelItem> FileModelItem;
typedef CodeModelPointer<_FunctionDefinitionModelItem> FunctionDefinitionModelItem;
typedef CodeModelPointer<_FunctionModelItem> FunctionModelItem;
typedef CodeModelPointer<_NamespaceModelItem> NamespaceModelItem;
typedef CodeModelPointer<_ScopeModelItem> ScopeModelItem;
typedef CodeModelPointer<_TemplateParameterModelItem> TemplateParameterModelItem;
typedef CodeModelPointer<_TypeAliasModelItem> TypeAliasModelItem;
typedef CodeModelPointer<_VariableModelItem> VariableModelItem;
typedef CodeModelPointer<_MemberModelItem> MemberModelItem;

typedef QList<ArgumentModelItem> ArgumentList;
typedef QList<ClassModelItem> ClassList;
typedef QList<CodeModelItem> ItemList;
typedef QList<EnumModelItem> EnumList;
typedef QList<EnumeratorModelItem> EnumeratorList;
typedef QList<FileModelItem> FileList;
typedef QList<FunctionDefinitionModelItem> FunctionDefinitionList;
typedef QList<FunctionModelItem> FunctionList;
typedef QList<NamespaceModelItem> NamespaceList;
typedef QList<ScopeModelItem> ScopeList;
typedef QList<TemplateParameterModelItem> TemplateParameterList;
typedef QList<TypeAliasModelItem> TypeAliasList;
typedef QList<VariableModelItem> VariableList;
typedef QList<MemberModelItem> MemberList;

#endif // CODEMODEL_FWD_H
