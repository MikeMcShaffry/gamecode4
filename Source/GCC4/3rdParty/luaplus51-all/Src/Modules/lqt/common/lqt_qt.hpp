#ifndef __LQT_QT_HPP
#define __LQT_QT_HPP

#include "lqt_common.hpp"
#include <QDateTime>
#include <QPen>
#include <QBrush>
#include <QVariant>
#include <QFontMetrics>
#include <QFont>
#include <QLocale>
#include <QPalette>
#include <QIcon>
#include <QTextCharFormat>
#include <QModelIndex>
#include <QDir>
#include <QCursor>
#include <QTextCursor>
#include <QTextOption>
#include <QBitmap>
#include <QItemSelection>
#include <QTextDocumentFragment>
#include <QTextBlock>
#include <QUrl>
#include <QBitArray>

int lqtL_qt_metacall (lua_State *, QObject *, QObject *, QMetaObject::Call, const char *, int, void **);
void lqtL_qobject_custom (lua_State *L);

// custom type handlers

QList<QByteArray> lqtL_getStringList(lua_State *L, int i);
void lqtL_pushStringList(lua_State *L, const QList<QByteArray> &table);

#endif // __LQT_QT_HPP


