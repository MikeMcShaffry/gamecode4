/*
 * Copyright (c) 2007-2008 Mauro Iazzi
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

#include "common_bind.hpp"
#include "luafunction.hpp"
#include "lqt_bind_QLabel.hpp"
#include "lqt_bind_QCoreApplication.hpp"
#include "lqt_bind_QApplication.hpp"
#include "lqt_bind_QAbstractButton.hpp"
#include "lqt_bind_QPushButton.hpp"
#include "lqt_bind_QObject.hpp"
#include "lqt_bind_QFont.hpp"
#include "lqt_bind_QWidget.hpp"
#include "lqt_bind_QTextEdit.hpp"
#include "lqt_bind_QAbstractScrollArea.hpp"
#include "lqt_bind_QFrame.hpp"
#include "lqt_bind_QEvent.hpp"
#include "lqt_bind_QLayout.hpp"
#include "lqt_bind_QBoxLayout.hpp"
#include "lqt_bind_QVBoxLayout.hpp"

#define EXPORT(n) do { \
	luaL_getmetatable(L,#n "*" );\
	lua_setglobal(L, #n);\
} while (0)

/*
#include <QApplication>
#include <QPushButton>
#include <QDebug>
#include <QSlider>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QShortcut>
#include <QWidget>
// */

static int retriever (lua_State *L) {
	lua_pushvalue(L, lua_upvalueindex(1));
	return 1;
}

static int F2S (lua_State *L) {
	lua_settop(L, 1);
	if (!lua_isfunction(L, -1)) {
		return 0;
	}
	LuaFunction *lf = new LuaFunction(L);
	lua_pushstring(L, "1function()");
	return 2;
}

int main(int argc, char *argv[])
{
  QApplication *app = new QApplication(argc, argv);

  int st = 0;
	int oldtop = 0;
  lua_State *L = lua_open();
  luaL_openlibs(L);
  luaopen_QEvent(L);
  luaopen_QObject(L);
  luaopen_QFont(L);
  luaopen_QWidget(L);
  luaopen_QLabel(L);
  luaopen_QPushButton(L);
  luaopen_QAbstractButton(L);
  luaopen_QApplication(L);
  luaopen_QCoreApplication(L);
  luaopen_QTextEdit(L);
  luaopen_QAbstractScrollArea(L);
  luaopen_QFrame(L);

	luaopen_QLayout(L);
	luaopen_QBoxLayout(L);
	luaopen_QVBoxLayout(L);

	lua_pushcfunction(L, F2S);
	lua_setglobal(L, "SLOT");

#if 1
	EXPORT(QWidget);
	EXPORT(QObject);
	EXPORT(QLabel);
	EXPORT(QFont);
	EXPORT(QEvent);
	EXPORT(QAbstractButton);
	EXPORT(QPushButton);
	EXPORT(QApplication);
	EXPORT(QTextEdit);
	EXPORT(QVBoxLayout);

	lua_pushvalue(L, LUA_REGISTRYINDEX);
	lua_setglobal(L, "_R");
	luaL_dostring(L, "\
			do\
			local del = QEvent.delete\
			local done = {}\
			local search\
			function search (k, v)\
			if v == del then print ('here', v) return true end\
			if type(v) == 'table' then\
			  if not done[v] then\
				done[v]=true\
				for i, e in pairs(v) do\
				  if search(i, e) then print (k) end\
				end\
				end\
			end\
			end\
			search('_G', _G)\
			end");


	lqtL_pushudata(L, app, "QApplication*");
	lua_pushcclosure(L, retriever, 1);
	lua_setglobal(L, "app");

  lua_getglobal(L, "QApplication");
  lua_getglobal(L, "app");
	lua_setfield(L, -2, "new");

	lua_pushnil(L);
	lua_setglobal(L, "app");
	
	
	luaL_dofile(L, "tutorial.lua");
	qDebug() << lua_tostring(L, -2) << lua_tostring(L, -1);

	lua_getglobal(L, "PB");

#else
  lua_checkstack(L, 15);

  QWidget *mainwin = new LuaBinder<QWidget>(L, 0, 0);

  lqtL_pushudata(L, app, "QApplication*");
	lua_setglobal(L, "app");

  QPushButton butt;
	lqtL_pushudata(L, &butt, "QPushButton*");
	lua_setglobal(L, "my_button");
	luaL_dostring(L, "my_button:show();my_button:connect('2pressed()', my_button, '1close()')");
	luaL_dostring(L, "my_button:setWindowTitle'lqt ROCKS!'");
	luaL_dostring(L, "my_button:setText'Hello World'");
	luaL_dostring(L, "my_button:setFixedSize(200, 100)");
	qDebug() << lua_tostring(L, -1);
	lua_settop(L, 0);

  //QWidget *mainwin = new QWidget(0, 0);
	
	luaL_dostring(L, "function override(t, k) local old_t = t[k] if type(old_t)~='function' then return end t[k] = function(...) print('overridden ', k, 'on', ...) return old_t(...) end end");
	
  lqtL_pushudata(L, mainwin, "QWidget*");
  lua_pushvalue(L, -1);
  lua_setglobal(L, "mainwin");
	luaL_dostring(L, "my_button:show();my_button:connect('2pressed()', mainwin, '1close()')");
	
	oldtop = lua_gettop(L);
	//luaL_dostring(L, "print(mainwin.isWidgetType)");
	luaL_dostring(L, "print(mainwin:isWidgetType())");
	luaL_dostring(L, "mainwin:setWindowTitle'TestTitle'");
	qDebug() << lua_tostring(L, -1);
	lua_settop(L, oldtop);

	luaL_dostring(L, "override(mainwin, '__gc')");
	//luaL_dostring(L, "override(mainwin, '__index')");
	//luaL_dostring(L, "override(mainwin, '__newindex')");
	//luaL_dostring(L, "mainwin.qt_metacast = nil");
	//luaL_dostring(L, "print(mainwin.qt_metacast)");

  luaL_dostring(L, "mainwin['~QWidget'] = function(obj) print('deleting', obj) end;");
  //luaL_dostring(L, "local qtmc = mainwin.qt_metacast;"
                   //"mainwin.qt_metacast = function(...) print('overridden qt_metacast', ...) return qtmc(...) end;");
  //luaL_dostring(L, "local hfw = mainwin.heightForWidth;"
                   //"mainwin.heightForWidth = function(...) print('overridden heightForWidth', ...) return hfw(...) end;");
  //luaL_dostring(L, "print(mainwin.RenderFlag.IgnoreMask)");

  QVBoxLayout *qvbl = new QVBoxLayout;
  QTextEdit *qte = new QTextEdit;
  QSpinBox *qsb = new QSpinBox;
  qvbl->addWidget(qte);
  qvbl->addWidget(qsb);
  
  mainwin->setLayout(qvbl);
  
  qDebug() << "==============";

  lua_getfield(L, -1, "show");
  lua_pushvalue(L, -2);
  lua_call(L, 1, 0);

  lua_getfield(L, -1, "width");
  lua_pushvalue(L, -2);
  lua_call(L, 1, 1);
  qDebug() << lua_tonumber(L, -1) << mainwin->width();
  lua_pop(L, 1);

  lua_getfield(L, -1, "height");
  lua_pushvalue(L, -2);
  lua_call(L, 1, 1);
  qDebug() << lua_tonumber(L, -1) << mainwin->height();
  lua_pop(L, 1);

  lua_getfield(L, -1, "isTopLevel");
  lua_pushvalue(L, -2);
  lua_call(L, 1, 1);
  qDebug() << (bool)lua_toboolean(L, -1) << mainwin->isTopLevel();
  lua_pop(L, 1);

  qDebug() << lua_gettop(L) << lua_tostring(L, 2);

  lua_getfield(L, -1, "heightForWidth");
  lua_pushvalue(L, -2);
  lua_pushinteger(L, 278);
  lua_call(L, 2, 1);
  qDebug() << lua_tonumber(L, -1) << mainwin->heightForWidth(278);
  lua_pop(L, 1);

  lua_getfield(L, -1, "new");
  lua_call(L, 0, 1);
  lua_getfield(L, -1, "show");
  lua_pushvalue(L, -2);
  lua_call(L, 1, 0);
  lqtL_manageudata(L, -1);
  lua_pop(L, 1);

  //luaL_dostring(L, "child = mainwin.new();child:delete();");
  lua_gc(L, LUA_GCCOLLECT, 0);
  qDebug() << "=================";
  luaL_dostring(L, "w = mainwin.new();w:show();w:move(400, 400)");
  lua_gc(L, LUA_GCCOLLECT, 0);
  qDebug() << "=================";

  lua_getfield(L, -1, "move");
  lua_pushvalue(L, -2);
  lua_pushinteger(L, 100);
  lua_pushinteger(L, 100);
  lua_call(L, 3, 0);

  //mainwin->show();

  //delete mainwin;
  
  //st = app->exec();
	luaL_dostring(L, "app:exec()");
  
  qDebug() << "I will now close Lua state";
#endif
  lua_close(L);
  
  return st;
}

