#!/usr/bin/lua
--[[

Copyright (c) 2007-2008 Mauro Iazzi

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


xml = dofile'xml.lua'
B = dofile'binder.lua'

function NOINSTANCE(b, name)
  b.types_from_stack[name] = function(i) error('cannot copy '..name) end
  b.types_test[name] = function(i) error('cannot copy '..name) end
  b.types_to_stack[name] = function(i) error('cannot copy '..name) end

  b.types_from_stack[(name..' *')] = function(i) return '*static_cast<'..name..'**>(lqtL_toudata(L, '..tostring(i)..', "'..name..'"))' end
  b.types_test[(name..' *')] = function(i) return 'lqtL_testudata(L, '..tostring(i)..', "'..name..'*")' end
  b.types_to_stack[(name..' *')] = function(i) return 'lqtL_pushudata(L, '..tostring(i)..', "'..name..'*")' end
end

function cp_file(src, dst)
  do
		local src = (type(src)=='string') and io.open(src, 'r') or src
		local check = (type(dst)=='string') and io.open(dst, 'r') or nil
		if type(check)=='userdata' then
			local a, b = src:read'*a', check:read'*a'
			if a==b then check:close() src:close() return end
			check:close()
		end
	end
  src = (type(src)=='string') and io.open(src, 'r') or src
  dst = (type(dst)=='string') and io.open(dst, 'w') or dst
	local content = src:read('*a')
	dst:write(content)
	src:close()
	dst:close()
end

function BINDQT(n)
  n = tostring(n)
  local h, c = B:make_namespace(n, n, 'QtCore', 'QtGui')
  print(n..': writing definition file')
  f = io.open('src/lqt_bind_'..n..'.cpp', 'w')
  f:write(c)
  f:close()

  print(n..': writing prototypes file')
  f = io.open('src/lqt_bind_'..n..'.hpp', 'w')
  f:write(h)
  f:close()
end

function init_qt(B)
	B.filter = function (m)
		local n = type(m)=='table' and type(m.attr)=='table' and m.attr.name
		if n and string.match(n, "[_%w]*[xX]11[_%w]*$") then
			return true, 'it is X11 specific'
		end
		if n and string.match(n, "[_%w]*_[_%w]*$") then
			return true, 'it is meant to be internal'
		end
		return false
	end


	NOINSTANCE(B, 'QCoreApplication')


	B.types_from_stack['const QString&'] = function(i) return 'QString::fromAscii(lua_tostring(L, '..tostring(i)..'), lua_objlen(L, '..tostring(i)..'))' end
	B.types_test['const QString&'] = function(i) return '(lua_type(L, ' .. tostring(i) .. ')==LUA_TSTRING)' end
	B.types_to_stack['const QString&'] = function(i) return 'lua_pushlstring(L, '..tostring(i)..'.toAscii().data(), '..tostring(i)..'.toAscii().size())' end
	B.types_from_stack['QString'] = function(i) return 'QString::fromAscii(lua_tostring(L, '..tostring(i)..'), lua_objlen(L, '..tostring(i)..'))' end
	B.types_test['QString'] = function(i) return '(lua_type(L, ' .. tostring(i) .. ')==LUA_TSTRING)' end
	B.types_to_stack['QString'] = function(i) return 'lua_pushlstring(L, '..tostring(i)..'.toAscii().data(), '..tostring(i)..'.toAscii().size())' end

	B.types_from_stack['QByteArray'] = function(i) return 'QByteArray(lua_tostring(L, '..tostring(i)..'), lua_objlen(L, '..tostring(i)..'))' end
	B.types_test['QByteArray'] = function(i) return '(lua_type(L, ' .. tostring(i) .. ')==LUA_TSTRING)' end
	B.types_to_stack['QByteArray'] = function(i) return 'lua_pushlstring(L, '..tostring(i)..'.data(), '..tostring(i)..'.size())' end
end

function make_tree (cl, tf)
	f = io.open(tf..'.cpp', 'w')
	for n in pairs(cl) do
		f:write('#include <'..n..'>\n')
	end
	f:write'\nmain() {\n'
	for n in pairs(cl) do
		if not ({Qt=true})[n] then f:write('  '..n..' *'..string.lower(n)..';\n') end
	end
	f:write'}\n'
	f:close()
	os.execute('gccxml `pkg-config QtGui QtCore --cflags` -fxml='..tf..'.xml '..tf..'.cpp')
	--os.execute'gccxml -g -Wall -W -D_REENTRANT -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED -I/usr/share/qt4/mkspecs/linux-g++ -I. -I/usr/include/qt4/QtCore -I/usr/include/qt4/QtCore -I/usr/include/qt4/QtGui -I/usr/include/qt4/QtGui -I/usr/include/qt4 -I. -I. -I. -fxml=auto.xml auto.cpp'
	--os.remove(tf..'.cpp')
end

function make_standard_qt(B, classlist)
	print'copying common files'
  cp_file('lqt_qt_utils.hpp', 'src/lqt_qt_utils.hpp')
  cp_file('lqt_qt_utils.cpp', 'src/lqt_qt_utils.cpp')
  cp_file('lqt_common.hpp', 'src/lqt_common.hpp')
  cp_file('lqt_common.cpp', 'src/lqt_common.cpp')

	do
		local clist = {}
		for s in string.gmatch(classlist, '([%u%l%d]+)') do
			clist[s] = true
		end
		classlist = clist
	end

  local tmpfile='tmp/auto'

	make_tree(classlist, tmpfile)

	B:init(tmpfile..'.xml')
	init_qt(B)

	do
		local clist = {}
		for n in pairs(classlist) do
			local c = B:find_name(n)
			clist = B.set_union(clist, B:tree_of_bases(c))
		end
		classlist = B.set_union(classlist, clist)
	end

	for n in pairs(classlist) do
		BINDQT(n)
	end
end

function make_single_qt(B, class)
	local classlist = { class }

  local tmpfile='tmp/auto'

	make_tree(classlist, tmpfile)
	B:init(tmpfile..'.xml')
	init_qt(B)

	BINDQT(class)
end

function B:enum_push_body_plus_qt(id, c)
	local enum = (type(id)=='string') and self:find_id(id) or id
	local e_static = (self:find_id(enum.attr.context).tag == 'Class') and 'static ' or ''
	local e_context = self:context_name(enum)
	local e_name = 'lqt_pushenum_' .. enum.attr.name
	local e_proto, e_def = '', ''

	e_proto = e_proto .. '  ' .. e_static .. self.lua_proto(e_name) .. ';\n'
	e_def = e_def .. self.lua_proto(c .. e_name) .. ' '
	e_def = e_def .. '{\n'
	e_def = e_def .. '  int enum_table = 0;\n'
	e_def = e_def .. '  lua_getfield(L, LUA_REGISTRYINDEX, LQT_ENUMS);\n'
	e_def = e_def .. '  if (!lua_istable(L, -1)) {\n'
	e_def = e_def .. '    lua_pop(L, 1);\n'
	e_def = e_def .. '    lua_newtable(L);\n'
	e_def = e_def .. '    lua_pushvalue(L, -1);\n'
	e_def = e_def .. '    lua_setfield(L, LUA_REGISTRYINDEX, LQT_ENUMS);\n'
	e_def = e_def .. '  }\n'

	e_def = e_def .. '  lua_newtable(L);\n'
	e_def = e_def .. '  enum_table = lua_gettop(L);\n'
	for i, e in ipairs(enum) do
		if (type(e)=='table') and (e.tag=='EnumValue') then
			e_def = e_def .. '  lua_pushstring(L, "' .. e.attr.name .. '");\n'
			e_def = e_def .. '  lua_rawseti(L, enum_table, ' .. e.attr.init .. ');\n'
			e_def = e_def .. '  lua_pushinteger(L, ' .. e.attr.init .. ');\n'
			e_def = e_def .. '  lua_setfield(L, enum_table, "' .. e.attr.name .. '");\n'
		end
	end
	e_def = e_def .. '  lua_pushcfunction(L, ' .. c .. e_name .. '_QFLAGS_CREATOR' .. ');\n'
	e_def = e_def .. '  lua_setfield(L, enum_table, "QFlags");\n'
	e_def = e_def .. '  lua_pushvalue(L, -1);\n'
	e_def = e_def .. '  lua_setfield(L, -3, "' .. e_context .. enum.attr.name .. '");\n'
	e_def = e_def .. '  lua_remove(L, -2);\n'
	e_def = e_def .. '  return 1;\n'
	e_def = e_def .. '}\n'
  -- ######## QFLAGS SPECIFIC
	e_proto = e_proto .. '  ' .. e_static .. self.lua_proto(e_name..'_QFLAGS_CREATOR') .. ';\n'
	e_def = e_def .. self.lua_proto(c .. e_name .. '_QFLAGS_CREATOR') .. [[ {
	int argn = lua_gettop(L);
	int i = 0;
  void *p  = lua_newuserdata(L, sizeof(QFlags<]]..e_context..enum.attr.name..[[>*) + sizeof(QFlags<]]..e_context..enum.attr.name..[[>));
  QFlags<]]..e_context..enum.attr.name..[[> *fl = static_cast<QFlags<]]..e_context..enum.attr.name..[[>*>( static_cast<void*>(&static_cast<QFlags<]]..e_context..enum.attr.name..[[>**>(p)[1]) );
	*(void**)p = fl;
	for (i=1;i<=argn;i++) {
    *fl |= static_cast<]]..e_context..enum.attr.name..[[>(lqtL_toenum(L, i, "]]..e_context..enum.attr.name..[["));
	}
	if (luaL_newmetatable(L, "QFlags<]]..e_context..enum.attr.name..[[>*")) {
		lua_pushstring(L, "QFlags<]]..e_context..enum.attr.name..[[>*");
		lua_setfield(L, -2, "__qtype");
	}
	lua_setmetatable(L, -2);
	return 1;
}
]]
	--print (e_def)
	return e_proto, e_def, e_name
end

B.enum_push_body = B.enum_push_body_plus_qt


make_standard_qt(B,
--'QTimer'
[[
QAbstractScrollArea
QAction
QApplication
QBoxLayout
QColor
QCoreApplication
QDialog
QEvent
QFileDialog
QFont
QFrame
QGraphicsItem
QGraphicsScene
QGraphicsView
QHBoxLayout
QIcon
QImage
QInputEvent
QKeyEvent
QKeySequence
QLabel
QLayout
QLayoutItem
QLineEdit
QMainWindow
QMenuBar
QMenu
QMessageBox
QObject
QPaintDevice
QShortcut
QSize
QSyntaxHighlighter
Qt
QTextBlockUserData
QTextCharFormat
QTextCursor
QTextDocument
QTextEdit
QTextFormat
QToolBar
QVBoxLayout
QWidget
]]
)


