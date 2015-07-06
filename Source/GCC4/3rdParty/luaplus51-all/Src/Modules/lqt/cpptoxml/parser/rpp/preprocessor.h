/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
** Copyright 2005 Harald Fernengel <harry@kdevelop.org>
**
** This file is part of $PRODUCT$.
**
** $CPP_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

class QByteArray;
class PreprocessorPrivate;

class Preprocessor
{
public:
    Preprocessor();
    ~Preprocessor();

    void processFile(const QString &fileName);
    void processString(const QByteArray &str);

    void addIncludePaths(const QStringList &includePaths);

    QByteArray result() const;

    QStringList macroNames() const;

    struct MacroItem
    {
        QString name;
        QStringList parameters;
        QString definition;
        bool isFunctionLike;
#ifdef PP_WITH_MACRO_POSITION
        QString fileName;
#endif
    };
    QList<MacroItem> macros() const;

private:
    Q_DISABLE_COPY(Preprocessor)
    PreprocessorPrivate *d;
};

#endif
