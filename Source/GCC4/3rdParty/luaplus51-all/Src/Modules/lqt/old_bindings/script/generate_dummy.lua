#!/usr/bin/lua
--[[

Copyright (c) 2007 Mauro Iazzi

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

--]]


HEADER_DEFINE = tostring(os.getenv'HEADER_DEFINE' or '__LQT_DUMMY')
ARG_MAX = tonumber(os.getenv'ARG_MAX' or 2)
TYPES = { bool='lua_pushboolean', int='lua_pushinteger', double='lua_pushnumber', ['const char *']='lua_pushstring' }


io.write("#ifndef "..HEADER_DEFINE.."\n")
io.write("#define "..HEADER_DEFINE.."\n")


io.write[[


#include "lqt.hpp"

#include <QObject>
#include <QEvent>
#include <QDebug>
#include <QMoveEvent>
#include <QPoint>

#define LUA_DUMMY_REGISTRY "LuaDummy"
#define SEE_STACK(L, j) for (int j=1;j<=lua_gettop(L);j++) { qDebug() << j << '=' << luaL_typename(L, j) << '@' << lua_topointer (L, j); }

class LuaDummy: public QObject {
  Q_OBJECT

  public:
  LuaDummy(lua_State *state):L(state) {
    int dummyTable = lua_gettop(L);
    qDebug() << "Dummy" << this << "is born";
    lua_getfield(L, LUA_REGISTRYINDEX, LUA_DUMMY_REGISTRY);
    if (lua_isnil(L, -1)) {
      lua_pop(L, 1);
      lua_newtable(L);
      lua_pushvalue(L, -1);
      lua_setfield(L, LUA_REGISTRYINDEX, LUA_DUMMY_REGISTRY);
    }
    lua_insert(L, -2);

    lua_pushvalue(L, -1);
    lua_gettable(L, dummyTable);

    if (!lua_isqobject(L, -1)) {
      qDebug() << "not QObject* is" << luaL_typename(L, -1);
      lua_pop(L, 1);
      // top of stack is the function I want
      qDebug() << "to be bound is" << luaL_typename(L, -1);
      lua_pushlightuserdata(L, this);
      lua_pushvalue(L, -2);
      lua_settable(L, dummyTable);
      // registry this is associated to this function
      lua_pushqobject(L, this);
      lua_insert(L, -2);
      lua_pushvalue(L, -2);
      lua_settable(L, dummyTable);
    } else {
      // leave the qobject on top;
      qDebug() << "Dummy" << this << "scheduled for deletion";
      this->deleteLater();
    }
    lua_replace(L, dummyTable);
    lua_settop(L, dummyTable);

    lua_newtable(L);
    lua_pushcfunction(L, __gc);
    lua_setfield(L, -2, "__gc");
    lua_setfenv(L, -2);
  }
  virtual ~LuaDummy() {
    qDebug() << "Dummy" << this << "is dead";
  }

  private:
    lua_State *L;

    static int __gc (lua_State *L) {
      QPointer<QObject> *qp = (QPointer<QObject> *)lua_touserdata(L, 1);
      if (*qp) (*qp)->deleteLater();
	qDebug() << "LuaDummy" << *qp << "scheduled for deletion";
	return 0;
      }
  protected:
  public:
  
    virtual bool eventFilter (QObject * watched, QEvent * event) {
      bool ret = false;
      int dummyTable = lua_gettop(L) + 1;
      lua_getfield(L, LUA_REGISTRYINDEX, LUA_DUMMY_REGISTRY);
      qDebug() << "dummyTable =" << dummyTable;
      SEE_STACK(L, k);qDebug() << "====";
      if (!lua_istable(L, -1)) {
        lua_settop(L, dummyTable-1);
        return false;
      }
      SEE_STACK(L, j);qDebug() << "====";
      lua_pushlightuserdata(L, this);
      SEE_STACK(L, j);qDebug() << "====";
      lua_gettable(L, dummyTable);
      SEE_STACK(L, j);qDebug() << "====";
      lua_pushqobject(L, watched);
      SEE_STACK(L, j);qDebug() << "====";
      
      switch (event->type()) {
      case QEvent::None: lua_pushnil(L); break;
      case QEvent::AccessibilityDescription: break;
      case QEvent::AccessibilityHelp: break;
      case QEvent::AccessibilityPrepare: lua_pushstring(L, "AccessibilityPrepare"); break;
      case QEvent::ActionAdded: break;
      case QEvent::ActionChanged: break;
      case QEvent::ActionRemoved: break;
      case QEvent::ActivationChange: break;
      case QEvent::ApplicationActivate: break;
      case QEvent::ApplicationDeactivate: break;
      case QEvent::ApplicationFontChange: break;
      case QEvent::ApplicationLayoutDirectionChange: break;
      case QEvent::ApplicationPaletteChange: break;
      case QEvent::ApplicationWindowIconChange: break;
      case QEvent::ChildAdded: break;
      case QEvent::ChildPolished: break;
      case QEvent::ChildRemoved: break;
      case QEvent::Clipboard: break;
      case QEvent::Close: break;
      case QEvent::ContextMenu: break;
      case QEvent::DeferredDelete: break;
      case QEvent::DragEnter: break;
      case QEvent::DragLeave: break;
      case QEvent::DragMove: break;
      case QEvent::Drop: break;
      case QEvent::EnabledChange: break;
      case QEvent::Enter: lua_pushstring(L, "Enter"); break;
      //case QEvent::EnterEditFocus: break;
      case QEvent::EnterWhatsThisMode: break;
      case QEvent::FileOpen: break;
      case QEvent::FocusIn: break;
      case QEvent::FocusOut: break;
      case QEvent::FontChange: break;
      case QEvent::GraphicsSceneContextMenu: break;
      case QEvent::GraphicsSceneDragEnter: break;
      case QEvent::GraphicsSceneDragLeave: break;
      case QEvent::GraphicsSceneDragMove: break;
      case QEvent::GraphicsSceneDrop: break;
      case QEvent::GraphicsSceneHelp: break;
      case QEvent::GraphicsSceneHoverEnter: break;
      case QEvent::GraphicsSceneHoverLeave: break;
      case QEvent::GraphicsSceneHoverMove: break;
      case QEvent::GraphicsSceneMouseDoubleClick: break;
      case QEvent::GraphicsSceneMouseMove: break;
      case QEvent::GraphicsSceneMousePress: break;
      case QEvent::GraphicsSceneMouseRelease: break;
      case QEvent::GraphicsSceneWheel: break;
      case QEvent::Hide: break;
      case QEvent::HideToParent: break;
      case QEvent::HoverEnter: break;
      case QEvent::HoverLeave: break;
      case QEvent::HoverMove:
        lua_pushstring(L, "HoverMove");
        lua_newtable(L);
        lua_pushinteger(L, static_cast<QHoverEvent*>(event)->oldPos().x()); lua_setfield(L, -2, "oldx");
        lua_pushinteger(L, static_cast<QHoverEvent*>(event)->oldPos().y()); lua_setfield(L, -2, "oldy");
        lua_pushinteger(L, static_cast<QHoverEvent*>(event)->pos().x()); lua_setfield(L, -2, "x");
        lua_pushinteger(L, static_cast<QHoverEvent*>(event)->pos().y()); lua_setfield(L, -2, "y");
        break;
      case QEvent::IconDrag: break;
      case QEvent::IconTextChange: break;
      case QEvent::InputMethod: break;
      case QEvent::KeyPress: break;
      case QEvent::KeyRelease: break;
      case QEvent::LanguageChange: break;
      case QEvent::LayoutDirectionChange: break;
      case QEvent::LayoutRequest: break;
      case QEvent::Leave: lua_pushstring(L, "Leave"); break;
      //case QEvent::LeaveEditFocus: break;
      case QEvent::LeaveWhatsThisMode: break;
      case QEvent::LocaleChange: break;
      case QEvent::MenubarUpdated: break;
      case QEvent::MetaCall: break;
      case QEvent::ModifiedChange: break;
      case QEvent::MouseButtonDblClick: break;
      case QEvent::MouseButtonPress: break;
      case QEvent::MouseButtonRelease: break;
      case QEvent::MouseMove: break;
      case QEvent::MouseTrackingChange: break;
      case QEvent::Move:
        lua_pushstring(L, "Move");
        lua_newtable(L);
        lua_pushinteger(L, static_cast<QMoveEvent*>(event)->oldPos().x()); lua_setfield(L, -2, "oldx");
        lua_pushinteger(L, static_cast<QMoveEvent*>(event)->oldPos().y()); lua_setfield(L, -2, "oldy");
        lua_pushinteger(L, static_cast<QMoveEvent*>(event)->pos().x()); lua_setfield(L, -2, "x");
        lua_pushinteger(L, static_cast<QMoveEvent*>(event)->pos().y()); lua_setfield(L, -2, "y");
        break;
      case QEvent::Paint: break;
      case QEvent::PaletteChange: break;
      case QEvent::ParentAboutToChange: break;
      case QEvent::ParentChange: break;
      case QEvent::Polish: break;
      case QEvent::PolishRequest: break;
      case QEvent::QueryWhatsThis: break;
      case QEvent::Resize: break;
      case QEvent::Shortcut: break;
      case QEvent::ShortcutOverride: break;
      case QEvent::Show: break;
      case QEvent::ShowToParent: break;
      case QEvent::SockAct: break;
      case QEvent::StatusTip: break;
      case QEvent::StyleChange: break;
      case QEvent::TabletMove: break;
      case QEvent::TabletPress: break;
      case QEvent::TabletRelease: break;
      case QEvent::TabletEnterProximity: break;
      case QEvent::TabletLeaveProximity: break;
      case QEvent::Timer: break;
      case QEvent::ToolBarChange: break;
      case QEvent::ToolTip: break;
      case QEvent::UpdateLater: break;
      case QEvent::UpdateRequest: break;
      case QEvent::WhatsThis: break;
      case QEvent::WhatsThisClicked: break;
      case QEvent::Wheel: break;
      case QEvent::WinEventAct: break;
      case QEvent::WindowActivate: break;
      case QEvent::WindowBlocked: break;
      case QEvent::WindowDeactivate: break;
      default:
        lua_pushstring(L, "<unknown>");
        break;
      }
      while (lua_gettop(L) < dummyTable+4) { lua_pushnil(L); }
      SEE_STACK(L, j);
      qDebug() << "try to call event filter; type =" << event->type();
      lua_call(L, 3, 1);
      qDebug() << "called event filter";
      ret = (bool)lua_toboolean(L, -1);
      lua_settop(L, dummyTable-1);
      SEE_STACK(L, k);qDebug() << "====";
      return ret;
  };
  // DO NOT ENABLE THIS: IT IS FATAL! however, guess why?
  //virtual bool event ( QEvent * event) { return eventFilter(this, event); }
  
  public slots:
]]

signatures = {
  [0] = {
    [''] = '',
  }
}

for nargs = 1,ARG_MAX do
  signatures[nargs] = {}
  for oldsig, oldbody in pairs(signatures[nargs-1]) do
    for argtype, argpush in pairs(TYPES) do
      signatures[nargs][oldsig..((nargs==1) and '' or ', ')..argtype..' arg'..tostring(nargs)] = oldbody..argpush.."(L, arg"..tostring(nargs)..');'
    end
  end
end

for i=0,ARG_MAX do
  for s, b in pairs(signatures[i]) do
    io.write'  void generic_slot ('
    io.write(s)
    io.write[[) {
      int dummyTable = lua_gettop(L) + 1;
      lua_getfield(L, LUA_REGISTRYINDEX, LUA_DUMMY_REGISTRY);
      if (!lua_istable(L, -1)) return;
      lua_pushlightuserdata(L, this);
      lua_gettable(L, dummyTable);
      ]]
    io.write(b)
    io.write'\n'
    io.write[[
      lua_call(L, ]]
    io.write(tostring(i))
    io.write', 0);\n'
    io.write[[
  };]]
    io.write'\n'
  end
end

io.write[[
};


]]

io.write("#endif // "..HEADER_DEFINE.."\n")

