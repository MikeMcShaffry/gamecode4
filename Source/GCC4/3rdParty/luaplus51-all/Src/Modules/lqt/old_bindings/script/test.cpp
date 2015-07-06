/*
 * Copyright (c) 2007 Mauro Iazzi
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "lqt.hpp"

#include <QApplication>
#include <QPushButton>
#include <QDebug>
#include <QSlider>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QShortcut>

#include "luadummy.hpp"

static int my_slot (lua_State *L) {
  qDebug() << "my_slot called with" << lua_gettop(L) << "arguments";
  if ( lua_gettop(L)==0 ) {
    lua_getglobal(L, "text_edit");
    lua_getfield(L, 1, "plainText");
    lua_remove(L, 1);
    luaL_dostring(L, lua_tostring(L, 1));
    lua_remove(L, 1);
  }
  QString mess("");
  for (int i=1;i<=lua_gettop(L);i++) {
    QVariant val = lua_toqvariant(L, i);
    mess.append(QString::number(i));
    mess.append(" => ");
    mess.append(val.toString());
    mess.append('\n');
  }
  QMessageBox::information(0, "Results", mess);
  return 0;
}

static int lua_pushshortcut(lua_State *L) {
	QWidget *parent = qobject_cast<QWidget*>(luaL_checkqobject(L, 1));
	QShortcut *sc = new QShortcut(parent);
	lua_pushqobject(L, sc);
	return 1;
}

static int lua_slot(lua_State *L) {
	lua_pushvalue(L, 1);
	QObject *dummy = new LuaDummy(L);
	lua_pushqobject(L, dummy);
	return 1;
}


int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  int st = 0;
  lua_State *L = lua_open();
  luaL_openlibs(L);

  lua_newtable(L);
  lua_pushcfunction(L, lua_pushshortcut);
  lua_setfield(L, -2, "new");
  lua_setglobal(L, "QShortcut");

  lua_pushcfunction(L, lua_slot);
  lua_setglobal(L, "SLOT");

  lua_newtable(L);
  lua_pushcfunction(L, lqt_connect);
  lua_setfield(L, -2, "connect");
  lua_setglobal(L, "qt");
  
  luaL_checkstack(L, 15, "shit");

//   QSlider *hello2 = new QSlider;
//   QSpinBox *hello = new QSpinBox;
//   hello->resize(100, 30);
//   hello->show();
//   hello2->resize(100, 100);
//   hello2->show();

//   lua_pushqobject(L, hello);
//   lua_pushqobject(L, hello);
    
//     lua_getfield(L, -1, "setValue");
//     lua_pushqobject(L, hello);
//     lua_pushinteger(L, 99);
//     lua_call(L, 2, 0);
    
//     lua_pushqobject(L, hello);
//     lua_pushstring(L, "suffix");
//     lua_pushstring(L, "suffix");
//     lua_settable(L, -3);
//     lua_pushstring(L, "prefix");
//     lua_pushstring(L, "prefix");
//     lua_settable(L, -3);
//     lua_getfield(L, -1, "cleanText");
//     qDebug() << lua_tostring(L, -1);
    
//     lua_pushcfunction(L, lqt_connect);
//     lua_pushqobject(L, hello2);
//     lua_pushstring(L, "valueChanged(int)");
//     lua_pushqobject(L, hello);
//     lua_pushstring(L, "setValue ( int ) ");
//     lua_call(L, 4, 1);
//     qDebug() << (bool)lua_toboolean(L, -1);
//     lua_pop(L, 1);
//     lua_pushcfunction(L, lqt_disconnect);
//     lua_pushqobject(L, hello);
//     lua_pushstring(L, "valueChanged(int)");
//     lua_pushqobject(L, hello2);
//     lua_pushstring(L, "setValue ( int ) ");
//     lua_call(L, 4, 1);
//     qDebug() << (bool)lua_toboolean(L, -1);
//     lua_pop(L, 1);
    
    
    // ???????
/*    lua_pushcfunction(L, lqt_connect);
    lua_pushqobject(L, &hello);
    lua_pushstring(L, "destroyed()");
    lua_pushqobject(L, &hello2);
    lua_pushstring(L, "deleteLater()");
    lua_call(L, 4, 1);
    qDebug() << (bool)lua_toboolean(L, -1);
    lua_pop(L, 1);
    lua_pushcfunction(L, lqt_connect);
    lua_pushqobject(L, &hello2);
    lua_pushstring(L, "destroyed()");
    lua_pushqobject(L, &hello);
    lua_pushstring(L, "deleteLater()");
    lua_call(L, 4, 1);
    qDebug() << (bool)lua_toboolean(L, -1);
    lua_pop(L, 1);
    QObject::connect(&hello, SIGNAL(destroyed()), &hello2, SLOT(deleteLater()));*/
    
    
/*    lua_pushqobject(L, b);
    lua_pop(L, 3);
    delete b;
    delete a;*/
  
  
//   lua_pushqobject(L, &hello);
//   QVariant var = lua_toqvariant(L, -1);
//   lua_pop(L, 1);
//   qDebug() << var.typeName();
  
  
  QPushButton *a = new QPushButton("Exec");
//   lua_pushqobject(L, a);
//   lua_getfield(L, -1, "close");
//   lua_pushqobject(L, a);
//   lua_call(L, 1, 1);
/*  qDebug() << lua_toqvariant(L, -1);*/
//   delete a;
//   lua_getfield(L, -1, "text");
//   qDebug() << lua_toqvariant(L, -1);
//   lua_pop(L, 1);
//   lua_pop(L, 1);
  
  QWidget mainwin;

  lua_pushqobject(L, &mainwin);
  lua_setglobal(L, "mainwin");

  QVBoxLayout *qvbl = new QVBoxLayout;
  QTextEdit *qte = new QTextEdit;
  QSpinBox *qsb = new QSpinBox;
  qvbl->addWidget(qte);
//   qvbl->addWidget(hello);
//   qvbl->addWidget(hello2);
  qvbl->addWidget(a);
  qvbl->addWidget(qsb);
  
  mainwin.setLayout(qvbl);
  mainwin.show();
  
//   lua_pushcfunction(L, methods_of);
  lua_pushqobject(L, qte);
  lua_setglobal(L, "text_edit");
//   lua_call(L, 1, 0);
  
  
  lua_pushcfunction(L, my_slot);
  int t = lua_gettop(L);
  
  
//   lua_pushcfunction(L, methods_of);
  lua_pushvalue(L, t);
  QObject * dummy2 = new LuaDummy(L);
//   lua_call(L, 1, 0);
  
  
  
  lua_pushcfunction(L, lqt_connect);
  lua_pushqobject(L, a);
  lua_pushstring(L, "pressed ()");
  
  lua_pushvalue(L, t);
  QObject * dummy1 = new LuaDummy(L);
  lua_pushstring(L, "generic_slot");
  lua_call(L, 4, 1);
  qDebug() << (bool)lua_toboolean(L, -1);
  lua_pop(L, 1);
  

  lua_pushcfunction(L, lqt_connect);
  lua_pushqobject(L, qsb);
  lua_pushstring(L, "valueChanged");
  
  lua_pushvalue(L, t);
  QObject * dummy3 = new LuaDummy(L);
  lua_pushstring(L, "generic_slot(int)");
  lua_call(L, 4, 1);
  qDebug() << (bool)lua_toboolean(L, -1);
  lua_pop(L, 1);
  
  QShortcut *sc = new QShortcut(&mainwin);
  lua_pushcfunction(L, lqt_connect);
  lua_pushqobject(L, sc);
  lua_pushstring(L, "Ctrl+P");
  lua_setfield(L, -2, "key");
  lua_pushstring(L, "activated()");
  lua_pushvalue(L, t);
  QObject * dummy4 = new LuaDummy(L);
  lua_pushstring(L, "generic_slot");
  lua_call(L, 4, 1);
  qDebug() << (bool)lua_toboolean(L, -1);
  lua_pop(L, 2);
  
  luaL_dostring(L, "return function (...) print(...) return true end");
  QObject * dummy5 = new LuaDummy(L);
  qsb->installEventFilter(dummy5);
  
  
  
//   return 0;
//   QMetaObject::invokeMethod(&hello, "setDisabled", Q_ARG(bool, true));
//   QGenericArgument a(Q_ARG(bool, true));
//   qDebug() << a.data() << a.name();
  
  
  st = app.exec();
  
  lua_close(L);
  
//   QObject *pippo = new QObject::QObject;
  
  return st;
}

