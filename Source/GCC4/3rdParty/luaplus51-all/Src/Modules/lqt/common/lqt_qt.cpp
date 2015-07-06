#include "lqt_qt.hpp"


int lqtL_qt_metacall (lua_State *L, QObject *self, QObject *acceptor,
        QMetaObject::Call call, const char *name,
        int index, void **args) {
    int callindex = 0, oldtop = 0;
    oldtop = lua_gettop(L);
    lqtL_pushudata(L, self, name); // (1)
    lua_getfield(L, -1, LQT_OBJSIGS); // (2)
    if (lua_isnil(L, -1)) {
        // TODO: determine what is wrong
        lua_settop(L, oldtop);
        QMetaObject::activate(self, self->metaObject(), index, args);
    } else {
        //qDebug() << lua_gettop(L) << luaL_typename(L, -1);
        lua_rawgeti(L, -1, index + 1); // (3)
        if (!lua_isstring(L, -1)) {
            lua_settop(L, oldtop);
            QMetaObject::activate(self, self->metaObject(), index, args);
        } else {
            callindex = acceptor->metaObject()->indexOfSlot(lua_tostring(L, -1));
            // qDebug() << "Found slot" << name << lua_tostring(L,-1) << "on" << acceptor->objectName() << "with index" << callindex;
            lua_pop(L, 2); // (1)
            lua_getfield(L, -1, LQT_OBJSLOTS); // (2)
            lua_rawgeti(L, -1, index+1); // (3)
            lua_remove(L, -2); // (2)
            index = acceptor->qt_metacall(call, callindex, args);
            lua_settop(L, oldtop);
        }
    }
    return -1;
}


const char add_method_func[] =
"return function(qobj, signature, func)\n"
"	local qname = 'LuaObject('..tostring(qobj)..')'\n"
"	local stringdata = qobj['"LQT_OBJMETASTRING"']\n"
"	local data = qobj['"LQT_OBJMETADATA"']\n"
"	local slots = qobj['"LQT_OBJSLOTS"']\n"
"	local sigs = qobj['"LQT_OBJSIGS"']\n"
"	if stringdata==nil then\n"
"		--print'adding a slot!'\n"
"		--initialize\n"
"		stringdata = qname..'\\0'\n"
"		data = setmetatable({}, {__index=table})\n"
"		data:insert(1) -- revision\n"
"		data:insert(0) -- class name\n"
"		data:insert(0) -- class info (1)\n"
"		data:insert(0) -- class info (2)\n"
"		data:insert(0) -- number of methods\n"
"		data:insert(10) -- beginning of methods\n"
"		data:insert(0) -- number of properties\n"
"		data:insert(0) -- beginning of properties\n"
"		data:insert(0) -- number of enums/sets\n"
"		data:insert(0) -- beginning of enums/sets\n"
"		slots = setmetatable({}, {__index=table})\n"
"		sigs = setmetatable({}, {__index=table})\n"
"	end\n"
"	local name, args = string.match(signature, '^(.*)(%b())$')\n"
"	local arg_list = ''\n"
"	if args=='()' then\n"
"		arg_list=''\n"
"	else\n"
"		local argnum = select(2, string.gsub(args, '.+,', ','))+1\n"
"		for i = 1, argnum do\n"
"			if i>1 then arg_list=arg_list..', ' end\n"
"			arg_list = arg_list .. 'arg' .. i\n"
"		end\n"
"	end\n"
"	--print(arg_list, signature)\n"
"	local sig, params = #stringdata + #arg_list + 1, #stringdata -- , ty, tag, flags\n"
"	stringdata = stringdata .. arg_list .. '\\0' .. signature .. '\\0'\n"
"	data:insert(sig) -- print(sig, string.byte(stringdata, sig, sig+4), string.char(string.byte(stringdata, sig+1, sig+6)))\n"
"	data:insert(params) -- print(params, string.char(string.byte(stringdata, params+1, params+10)))\n"
"	data:insert(#stringdata-1) -- print(#stringdata-1, (string.byte(stringdata, #stringdata)))\n"
"	data:insert(#stringdata-1) -- print(#stringdata-1, (string.byte(stringdata, #stringdata)))\n"
"	if func then\n"
"		data:insert(0x0a)\n"
"		slots:insert(func)\n"
"		sigs:insert('__slot'..signature:match'%b()')\n"
"	else\n"
"		data:insert(0x05)\n"
"		slots:insert(false)\n"
"		sigs:insert(false)\n"
"	end\n"
"	data[5] = data[5] + 1\n"
"	qobj['"LQT_OBJMETASTRING"'] = stringdata\n"
"	qobj['"LQT_OBJMETADATA"'] = data\n"
"	qobj['"LQT_OBJSLOTS"'] = slots\n"
"	qobj['"LQT_OBJSIGS"'] = sigs\n"
"end\n";

#include <QMetaObject>
#include <QMetaMethod>

#define CASE(x) case QMetaMethod::x : lua_pushstring(L, " " #x); break
static int lqtL_methods(lua_State *L) {
	QObject* self = static_cast<QObject*>(lqtL_toudata(L, 1, "QObject*"));
	if (self == NULL)
		return luaL_argerror(L, 1, "expecting QObject*");
	const QMetaObject *mo = self->metaObject();
	lua_createtable(L, mo->methodCount(), 0);
	for (int i=0; i < mo->methodCount(); i++) {
		QMetaMethod m = mo->method(i);
		lua_pushstring(L, m.signature());
		switch (m.access()) {
		CASE(Private);
		CASE(Protected);
		CASE(Public);
		}
		switch (m.methodType()) {
		CASE(Method);
		CASE(Signal);
		CASE(Slot);
		CASE(Constructor);
		}
		lua_concat(L, 3);
		lua_rawseti(L, -2, i+1);
	}
	return 1;
}
#undef CASE

static int lqtL_pushqobject(lua_State *L, QObject * object) {
    const QMetaObject * meta = object->metaObject();
    while (meta) {
        QString className = meta->className();
        className += "*";
        char * cname = strdup(qPrintable(className));
        lua_getfield(L, LUA_REGISTRYINDEX, cname);
        int isnil = lua_isnil(L, -1);
        lua_pop(L, 1);
        if (!isnil) {
            lqtL_pushudata(L, object, cname);
            free(cname);
            return 1;
        } else {
            free(cname);
            meta = meta->superClass();
        }
    }
    return 0;
}

static int lqtL_findchild(lua_State *L) {
    QObject* self = static_cast<QObject*>(lqtL_toudata(L, 1, "QObject*"));
    if (self == NULL)
        return luaL_argerror(L, 1, "expecting QObject*");

    QString name = luaL_checkstring(L, 2);
    QObject * child = self->findChild<QObject*>(name);

    if (child) {
        lqtL_pushqobject(L, child);
        return 1;
    } else {
        return 0;
    }
}

static int lqtL_children(lua_State *L) {
    QObject* self = static_cast<QObject*>(lqtL_toudata(L, 1, "QObject*"));
    if (self == NULL)
        return luaL_argerror(L, 1, "expecting QObject*");
    const QObjectList & children = self->children();

    lua_newtable(L);
    for (int i=0; i < children.count(); i++) {
        QObject * object = children[i];
        QString name = object->objectName();
        if (!name.isEmpty() && lqtL_pushqobject(L, object)) {
            lua_setfield(L, -2, qPrintable(name));
        }
    }
    return 1;
}

static int lqtL_connect(lua_State *L) {
    QObject* sender = static_cast<QObject*>(lqtL_toudata(L, 1, "QObject*"));
    if (sender == NULL)
        return luaL_argerror(L, 1, "sender not QObject*");
    const char *signal = luaL_checkstring(L, 2);
    QObject* receiver = static_cast<QObject*>(lqtL_toudata(L, 3, "QObject*"));
    if (receiver == NULL)
        return luaL_argerror(L, 3, "receiver not QObject*");
    const char *method = luaL_checkstring(L, 4);

    const QMetaObject *senderMeta = sender->metaObject();
    int idxS = senderMeta->indexOfSignal(signal + 1);
    if (idxS == -1)
        return luaL_argerror(L, 2, qPrintable(QString("no such sender signal: '%1'").arg(signal + 1)));

    const QMetaObject *receiverMeta = receiver->metaObject();
    int idxR = receiverMeta->indexOfMethod(method + 1);
    if (idxR == -1)
        return luaL_argerror(L, 4, qPrintable(QString("no such receiver method: '%1'").arg(method + 1)));

    bool ok = QObject::connect(sender, signal, receiver, method);
    lua_pushboolean(L, ok);
    return 1;
}

void lqtL_qobject_custom (lua_State *L) {
    lua_getfield(L, LUA_REGISTRYINDEX, "QObject*");
    int qobject = lua_gettop(L);

    lua_pushstring(L, "__addmethod");
    luaL_dostring(L, add_method_func);
    lua_rawset(L, qobject);

    lua_pushstring(L, "__methods");
    lua_pushcfunction(L, lqtL_methods);
    lua_rawset(L, qobject);

    lua_pushstring(L, "findChild");
    lua_pushcfunction(L, lqtL_findchild);
    lua_rawset(L, qobject);

    lua_pushstring(L, "children");
    lua_pushcfunction(L, lqtL_children);
    lua_rawset(L, qobject);

    lua_pushstring(L, "connect");
    lua_pushcfunction(L, lqtL_connect);
    lua_rawset(L, qobject);

    // also modify the static QObject::connect function
    lua_getfield(L, LUA_GLOBALSINDEX, "QObject");
    lua_pushcfunction(L, lqtL_connect);
    lua_setfield(L, -2, "connect");
}


QList<QByteArray> lqtL_getStringList(lua_State *L, int i) {
    QList<QByteArray> ret;
    int n = lua_objlen(L, i);
    for (int i=0; i<n; i++) {
        lua_pushnumber(L, i+1);
        lua_gettable(L, i);
        ret[i] = QByteArray(lua_tostring(L, -1));
        lua_pop(L, 1);
    }
    return ret;
}

void lqtL_pushStringList(lua_State *L, const QList<QByteArray> &table) {
    const int n = table.size();
    lua_createtable(L, n, 0);
    for (int i=0; i<n; i++) {
        lua_pushnumber(L, i+1);
        lua_pushstring(L, table[i].data());
        lua_settable(L, -3);
    }
}
