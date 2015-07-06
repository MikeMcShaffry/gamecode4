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
#include <cstring>
#include <QVariant>
#include <QPointer>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaType>
#include <QDebug>
#include <QGenericArgument>
#include <QPointer>

#ifdef QOBJECT_HANDLER
static int lqt_handler__gc (lua_State *L);
#endif // QOBJECT_HANDLER

static int lqt_qpointer__gc (lua_State *L);
static int method_index (const QMetaObject *M, QByteArray name, int start = 0);

// #define luaL_checkqobject(L, index) (*(QObject**)luaL_checkudata(L, index, QOBJECT_METATABLE))
// 
// int lua_pushqvariant (lua_State *L, QVariant *V);
// QVariant lua_toqvariant (lua_State *L, int index);

static int lua_isint (lua_State *L, int index) {
  int i = lua_tointeger(L, index);
  double d = lua_tonumber(L, index);
  return i==d;
}

static bool signal_slot_names (QObject *obj, QByteArray& name) {
  const QMetaObject *M = NULL;
  int index = -1;
  
  name = QMetaObject::normalizedSignature(name);
  
  M = obj->metaObject();
  index = M->indexOfMethod(name);
  if (index == -1) {
    return false;
  }
  switch (M->method(index).methodType()) {
    case QMetaMethod::Signal:
      name.prepend('2');
      break;
    case QMetaMethod::Slot:
      name.prepend('1');
      break;
    default:
      return false;
      break;
  };
  return true;
}

int lqt_connect (lua_State *L) {
  
  QObject *obj1 = luaL_checkqobject(L, 1);
  QByteArray func1 = QMetaObject::normalizedSignature(luaL_checkstring(L, 2));
  QObject *obj2 = luaL_checkqobject(L, 3);
  QByteArray func2 = QMetaObject::normalizedSignature(luaL_checkstring(L, 4));
  // check for QObjects validity
  
  const QMetaObject *M1 = obj1->metaObject();
  const QMetaObject *M2 = obj2->metaObject();
  int index1 = M1->indexOfMethod(func1);
  int index2 = M2->indexOfMethod(func2);
  
  if ( index1!=-1 && index2!=-1 ) {
    // nothing misses
  } else if ( index1!=-1 && index2==-1 ) {
    // slot is missing
    for (index2 = method_index(M2, func2, 0);index2!=-1;index2=method_index(M2, func2, index2+1)) {
      if ( (M2->method(index2).methodType()==QMetaMethod::Signal ||
            M2->method(index2).methodType()==QMetaMethod::Slot ) &&
            QMetaObject::checkConnectArgs(M2->method(index2).signature(), func1))
        break;
    }
    if (index2==-1) qDebug() << "no matching slot of name" << func2 << "for signal" << func1;
    else func2 = M2->method(index2).signature();
    // slot has been filled at best
  } else if ( index1==-1 && index2!=-1 ) {
    // signal is missing
    for (index1 = method_index(M1, func1, 0);index1!=-1;index1=method_index(M1, func1, index1+1)) {
      if (M1->method(index1).methodType()==QMetaMethod::Signal &&
          QMetaObject::checkConnectArgs(M1->method(index1).signature(), func2))
        break;
    }
    if (index1==-1) qDebug() << "no matching signal of name" << func1 << "for slot" << func2;
    else func1 = M1->method(index1).signature();
    // signal has been filled at best
  } else if ( index1==-1 && index2==-1 ) {
  }
  
  signal_slot_names(obj1, func1);
  signal_slot_names(obj2, func2);
  
//   if (!signal_slot_names(obj1, func1)) {
//     
//     if (!signal_slot_names(obj2, func2)) {
//     } else {
//       // find if a signal has same signature of slot
//       int m1 = -1;
//       for (m1 = method_index(M1, func1, 0);m1!=-1;m1=method_index(M1, func1, m1+1)) {
//         if (M1->method(m1).methodType()==QMetaMethod::Signal &&
//             QMetaObject::checkConnectArgs(M1->method(m1).signature(), func2))
//           break;
//       }
//       if (m1==-1) qDebug() << "no matching signal of name" << func1 << "for slot" << func2;
//     }
//     
//   } else {
//     if (!signal_slot_names(obj2, func2)) {
//       // find if a slot has same signature of signal
//       int m2 = -1;
//       for (m2 = method_index(M2, func2, 0);m2!=-1;m2=method_index(M2, func2, m2+1)) {
//         qDebug() << M2->method(m2).signature() << func1;
//         if ( (M2->method(m2).methodType()==QMetaMethod::Signal ||
//               M2->method(m2).methodType()==QMetaMethod::Slot ) &&
//               QMetaObject::checkConnectArgs(M2->method(m2).signature(), func1))
//           break;
//       }
//       if (m2==-1) qDebug() << "no matching slot of name" << func2 << "for signal" << func1;
//     } else {
//     }
//   }
  
  qDebug() << "trying to connect signal" << func1 << "to slot" << func2;
  lua_pushboolean(L, QObject::connect(obj1, func1, obj2, func2));
  
  return 1;
}

int lqt_disconnect (lua_State *L) {
  
  QObject *obj1 = luaL_checkqobject(L, 1);
  QByteArray func1= luaL_checkstring(L, 2);
  QObject *obj2 = luaL_checkqobject(L, 3);
  QByteArray func2= luaL_checkstring(L, 4);
  // check fo QObjects validity
  
  if (!signal_slot_names(obj1, func1)) {
    // TODO: handle method missing
  }
  if (!signal_slot_names(obj2, func2)) {
    // TODO: handle method missing
  }
  
  lua_pushboolean(L, QObject::disconnect(obj1, func1, obj2, func2));
  
  return 1;
}

static void method_see (const QMetaObject *M) {
  if (M->superClass()) method_see(M->superClass());
  qDebug() << M->className() << "methods:";
  for (int m=0;m<M->methodCount();m++) {
    qDebug() << "" << M->method(m).typeName() << M->method(m).signature();
  }
}

int methods_of (lua_State *L) {
  
  QObject *Q = luaL_checkqobject(L, 1);
  
  if (Q) {
    method_see(Q->metaObject());
  }
  
  return 0;
}


static int method_index (const QMetaObject *M, QByteArray name, int start) {
//   method_see(M);
//   qDebug() << name;
  int ret = M->indexOfMethod(name);
  
  if (ret >= start) { return ret; }
  
  name.append('(');
//   qDebug() << name;
  
  for (int m=start;m<M->methodCount();m++) {
    if ( QByteArray(M->method(m).signature()).startsWith(name) ) {
      ret = m;
      break;
    }
  }
  return ret;
}

// void delete_genericret (int type, void *data) {
//   QMetaType::destroy(type, data);
//   return;
// }

// QGenericReturnArgument lua_togenericret (int type, void **data) {
//   QGenericReturnArgument ret = QGenericReturnArgument();
//   *data = NULL;
//   *data = QMetaType::construct(type);
//   if (*data) ret = QGenericReturnArgument(QMetaType::typeName(type), *data);
//   return ret;
// }


// void delete_genericarg (lua_State *L, int index, void *data) {
//   if (!data) return;
//   QVariant var = lua_toqvariant(L, index);
//   QMetaType::destroy(var.type(), data);
//   return;
// }

#if 0
// TODO: should be done better (e.g. reentrant)
#define HANDLE_CASE_OF(T) case (QVariant:: T): { \
 static typeof(V.to##T ()) my##T = V.to##T (); return &my##T; \
} break
static const void *data_copy(const QVariant& V, int t) {
  switch (t) {
    HANDLE_CASE_OF(Bool);
    HANDLE_CASE_OF(Int);
    HANDLE_CASE_OF(Double);
    HANDLE_CASE_OF(String);
    default:
      break;
  }
  return (const void*)NULL;
}
#undef HANDLE_CASE_OF
#endif // 0

// QGenericArgument lua_togenericarg (lua_State *L, int index, void **data) {
// //   qDebug() << "passing QVariant";
//   QVariant var = lua_toqvariant(L, index);
//   *data = QMetaType::construct(var.type(), var.data());
//   QGenericArgument ret = QGenericArgument(var.typeName(), *data);
//   return ret;
// }

static int lqt_invoke (lua_State *L) {
  QVariant qvargs[10];
  QGenericArgument args[10];
  QGenericReturnArgument ret;
  QVariant qvret;
//   void *args_data[10];
//   void *ret_data;
//   int ret_type = -1;
  
  QObject *Q = luaL_checkqobject(L, 1);
  const QMetaObject *M = Q->metaObject();
  
  QByteArray method = luaL_checkstring(L, lua_upvalueindex(1));
  
//   ret_type =  QMetaType::type(luaL_checkstring(L, lua_upvalueindex(2)));
  
//   qDebug() << "calling" << method << "on object" << Q << "with return type" << QMetaType::typeName(ret_type);
//   QMetaObject::invokeMethod(Q, "show");
  
  for (int i=0;i<10;i++) {
    qvargs[i] = lua_toqvariant(L, i+2);
//     args[i] = lua_togenericarg(L, i+2, &args_data[i]);
  }
//   ret = lua_togenericret(ret_type, &ret_data);
  
  
  // DONE: find for sure which method the user wants
  int method_number = -1;
  int method_result = -1;
  for (int m = method_index(M, method, 0);m!=-1;m=method_index(M, method, m+1)) {
//     qDebug() << "checking for match" << M->method(m).signature();
    int corresponds = 0;
    QList<QByteArray> sig = M->method(m).parameterTypes();
    for (int i = 0; i < sig.size(); ++i) {
      QVariant::Type this_type = QVariant::nameToType(sig.at(i));
//       qDebug() << "checking for match" << sig.at(i) << qvargs[i].typeName();
      if (qvargs[i].type()==this_type) {
        corresponds += 21;
      } else if (qvargs[i].canConvert(this_type)) {
        corresponds += 20;
      } else {
        corresponds = -1;
        break;
      }
    }
    if (corresponds>method_result) {
      method_number = m;
      method_result = corresponds;
    }
  }
  if (method_number!=-1) {
    qDebug() << "I found best method is" << M->method(method_number).signature();
  }
  
  QMetaMethod mmeth = M->method(method_number);
  QByteArray method_name = mmeth.signature();
  method_name.truncate(method_name.indexOf('('));
//   qDebug() << method_name;
  QList<QByteArray> sig = mmeth.parameterTypes();
  for (int i = 0; i < sig.size(); ++i) {
    QVariant::Type this_type = QVariant::nameToType(sig.at(i));
    qvargs[i].convert(this_type);
    args[i] = QGenericArgument(qvargs[i].typeName(), qvargs[i].data());
  }
  
  
  qvret = QVariant(QVariant::nameToType(mmeth.typeName()));
  ret = QGenericReturnArgument(qvret.typeName(), qvret.data());
  qDebug() << "method call with ret=" << qvret;
  
  if ( QMetaObject::invokeMethod(Q, method_name, Qt::AutoConnection, ret,
       args[0], args[1], args[2], args[3], args[4],
       args[5], args[6], args[7], args[8], args[9] ) )
  {
    qDebug() << "method call returned:" << qvret;
//     QVariant r(ret_type, ret.data());
//     qDebug() << "method call returned:" << r;
    lua_pushqvariant(L, &qvret);
    return 1;
  } else {
    qDebug() << "method call failed";
  }
//   for (int i = 0;i<10;i++) {
//     delete_genericarg( L, i+2, args_data[i] );
//   }
//   delete_genericret( ret_type, ret_data );
  qDebug() << "method call finished";
  return 0;
}

// static int canConserve (const QVariant *V, QVariant::Type type) {
//   QVariant copy(*V);
//   QVariant::Type oldtype = copy.type();
//   if (!copy.canConvert(type)) return 0;
//   copy.convert(type);
//   if (!copy.canConvert(oldtype)) return 0;
//   copy.convert(oldtype);
//   return (copy==*V)?1:0;
// }


static int lqt__index (lua_State *L) {
  int index = -1;
  const char *name = NULL;
  size_t len = 0;
  
  QObject **qp = (QObject**)luaL_checkudata(L, 1, QOBJECT_METATABLE);
  QObject *Q = qp?*qp:NULL;
  
  // TODO: find if object has method overwritten
  
  name = lua_tolstring(L, 2, &len);
  qDebug() << "requested member: " << lua_tostring(L, 2);
  if (Q) {
    const QMetaObject *M = Q->metaObject();
    
//     for (int m=0;m<M->methodCount();m++){
//       qDebug() << M->method(m).signature();
//     }
    
    if ( (index=method_index(M, name, len)) != -1 ) {
      qDebug() << "method requested";
      lua_pushvalue(L, 2);
//       lua_pushstring(L, M->method(index).typeName());
      lua_pushcclosure(L, lqt_invoke, 1);
      return 1;
    } else if ( (index=M->indexOfProperty(name)) != -1) {
      QVariant value(M->property(index).read(Q));
      return lua_pushqvariant(L, &value);
    }/* else if ( (index=M->indexOfEnumerator(name)) != -1) {
      return 0;
    }*/
  } else {
    qDebug() << "NULL Object indexed";
    lua_pushnil(L);
    lua_pushstring(L, "QObject destroyed");
    return 2;
  }
  return 0;
}

static int lqt__newindex (lua_State *L) {
  int index = -1;
  const char *name = NULL;
  QVariant val;
  size_t len = 0;
  
  QObject **qp = (QObject**)luaL_checkudata(L, 1, QOBJECT_METATABLE);
  QObject *Q = qp?*qp:NULL;
  
  name = lua_tolstring(L, 2, &len);
  val = lua_toqvariant(L, 3);
  if (Q) {
    const QMetaObject *M = Q->metaObject();
    
    qDebug() << "setting property: " << name;
    
    index=M->indexOfProperty(name);
    if ( (index=M->indexOfProperty(name)) != -1) {
      if (!M->property(index).write(Q, val)) {
        lua_pushnil(L);
        lua_pushstring(L, "could not set property ");
        lua_pushstring(L, name);
        lua_pushstring(L, " for object of type ");
        lua_pushstring(L, M->className());
        lua_concat(L, 4);
        return 2;
      }
    } else {
//       qDebug() << "NULL Object assigned";
      lua_pushnil(L);
      lua_pushstring(L, "QObject does not have this property");
      return 2;
    }
  } else {
    lua_pushnil(L);
    lua_pushstring(L, "QObject destroyed");
    return 2;
  }
  return 0;
}

static struct luaL_Reg lqt_metatable[] = {
  { "__newindex", lqt__newindex },
  { "__index", lqt__index },
  { "__gc", lqt_qpointer__gc },
  { NULL, NULL },
};




/*************         HANDLER       ****************/

static int lqt_qpointer__gc (lua_State *L) {
  QPointer<QObject> *qp = (QPointer<QObject> *)lua_touserdata(L, 1);
  lua_getfenv(L, 1);
  if (lua_istable(L, -1)) {
    lua_getfield(L, -1, "__gc");
    if (lua_isfunction(L, -1)) {
      qDebug() << "found a __gc";
      lua_pushvalue(L, 1);
      lua_call(L, 1, 0);
    } else {
      lua_pop(L, 1);
    }
  }
  lua_pop(L, 1);
  *qp = NULL; // hoping this unsets the guard by Qt
//   qDebug() << "Garbage collecting qp =" << qp << ", *qp =" << *qp;
//   delete qp;
  return 0;
}

/*
 * This is __gc for the handler, which is not managed by Qt
 */
 
#ifdef QOBJECT_HANDLER
static int lqt_handler__gc (lua_State *L) {
  lua_getfield(L, LUA_REGISTRYINDEX, QOBJECT_HANDLER);
  QObject **qp1 = (QObject**)lua_touserdata(L, 1);
  QObject **qp2 = (QObject**)lua_touserdata(L, -1);
  if (qp1==qp2) {
    delete *qp1;
    *qp1 = NULL;
    qDebug() << "destroying handler";
  }
  return 0;
}

void QLuaHandler::destroyed (QObject * obj) {
  luaL_checkstack(L, 2, "Cannot grow stack to destroy QObject");
  lua_pushlightuserdata(L, obj);
  lua_gettable(L, LUA_REGISTRYINDEX);
  QObject **op = (QObject**)lua_touserdata(L, -1);
  qDebug() << "voiding object: " << *op;
  lua_pushlightuserdata(L, obj);
  lua_pushnil(L);
  lua_settable(L, LUA_REGISTRYINDEX);
  *op = NULL;
  lua_pop(L, 1);
}
#endif // QOBJECT_HANDLER

/*************   STACK FUNCTIONS   ****************/

int lua_pushqobject (lua_State *L, QObject *Q) {
//   QPointer<QObject> obj = Q;
  int ot = lua_gettop(L); /* old stack top */
//   qDebug() << "creating object: " << Q;
  
  // DONE: check stack
  luaL_checkstack(L, 3, "Cannot grow stack to push QObject");
  
  
  /*
  * if we already have it don't rebuild
  */
//   lua_pushlightuserdata(L, Q);
//   lua_gettable(L, LUA_REGISTRYINDEX);
//   if (lua_isqobject(L, -1)) {
//     return 1;
//   }
//   lua_pop(L, 1);
  
  /*
  * we don't have it: let's create.
  * push the new userdata (QObject*)
  */
  QPointer<QObject> *op = (QPointer<QObject> *)lua_newuserdata(L, sizeof(QPointer<QObject>));
  *op = NULL; /* if we fail later, do not leave invalid pointers */
//   qDebug() << "Creating qp =" << op << ", *qp =" << *op;
  /* insert in register indexed by its content */
//   lua_pushlightuserdata(L, Q);
//   lua_pushvalue(L, ot+1);
//   lua_settable(L, LUA_REGISTRYINDEX);

  /* get the metatable */
  if (luaL_newmetatable(L, QOBJECT_METATABLE)) {
    /* create metatable if it did not exist */
    // DONE: check validity of metatable
    luaL_register(L, NULL, lqt_metatable);
    lua_pushvalue(L, -1);
    lua_setmetatable(L, -2);/* it is its own metatable */
    lua_pushvalue(L, -1);
    lua_pushstring(L, QOBJECT_METATABLE);
    lua_settable(L, LUA_REGISTRYINDEX);
  }
  lua_setmetatable(L, -2);

#ifdef QOBJECT_HANDLER
  lua_getfield(L, LUA_REGISTRYINDEX, QOBJECT_HANDLER);
  // DONE: check presence of handler
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);
    QLuaHandler **qhp = (QLuaHandler **)lua_newuserdata(L, sizeof(*qhp));
    lua_newtable(L);
    lua_pushcfunction(L, lqt_handler__gc);
    lua_setfield(L, -2, "__gc");
    lua_setmetatable(L, -2);

    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, QOBJECT_HANDLER);
    *qhp = new QLuaHandler(L);
  }
  QObject *H = *(QObject**)lua_touserdata(L, -1);
  lua_pop(L, 1);
#endif // QOBJECT_HANDLER

  *op = Q;
  
#ifdef QOBJECT_HANDLER
  if (Q) QObject::connect( Q, SIGNAL(destroyed(QObject*)), H, SLOT(destroyed(QObject*)) );
#endif // QOBJECT_HANDLER

  return 1;
}

int lua_isqobject (lua_State *L, int index) {
  int ret = 0;
  if (lua_isuserdata(L, index)) {
    lua_getmetatable(L, index);
    luaL_getmetatable(L, QOBJECT_METATABLE);
    ret = lua_rawequal(L, -1, -2);
    lua_pop(L, 2);
  }
  return ret;
}

QObject *lua_toqobject (lua_State *L, int index) {
  if (lua_isqobject(L, index)) {
    QPointer<QObject> *qp = (QPointer<QObject> *)lua_touserdata(L, index);
    return *qp;
  }
  return NULL;
}

QVariant lua_toqvariant (lua_State *L, int index) {
  QVariant ret;
  switch (lua_type(L, index)) {
    case LUA_TNUMBER:
      if (lua_isint(L, index)) {
//         qDebug() << "getting int" << lua_tointeger(L, index);
        ret = QVariant((int)lua_tointeger(L, index));
      } else {
//         qDebug() << "getting double" << lua_tonumber(L, index);
        ret = QVariant(lua_tonumber(L, index));
      }
      break;
    case LUA_TBOOLEAN:
//       qDebug() << "getting bool" << (bool)lua_toboolean(L, index);
      ret = QVariant((bool)lua_toboolean(L, index));
      break;
    case LUA_TSTRING:
//       qDebug() << "getting string" << lua_tostring(L, index);
      ret = QVariant(lua_tostring(L, index));
      break;
    case LUA_TUSERDATA:
      lua_getmetatable(L, index);
      lua_gettable(L, LUA_REGISTRYINDEX);
//       qDebug() << "getting userdata";
      if (lua_isstring(L, -1)) {
        int type = QMetaType::type(lua_tostring(L, -1));
        lua_pop(L, 1);
        const void *obj = (const void*)lua_touserdata(L, index);
//         qDebug() << "getting userdata" << obj << "of type" << tname;
        ret = QVariant(type, obj);
      }
      break;
      break;
    case LUA_TNONE:
    case LUA_TNIL:
    case LUA_TTABLE:
    case LUA_TFUNCTION:
    case LUA_TTHREAD:
    case LUA_TLIGHTUSERDATA:
    default:
      break;
  };
  return ret;
}

int lua_pushqvariant (lua_State *L, QVariant *V) {
//   qDebug() << "pushing val: " << *V;
  switch (V->type()) {
    case QVariant::Bool:       lua_pushboolean(L, (int)V->toBool());       break;
    case QVariant::Int:        lua_pushinteger(L, V->toInt());        break;
    case QVariant::LongLong:   lua_pushinteger(L, V->toLongLong());   break;
    case QVariant::UInt:       lua_pushinteger(L, V->toUInt());       break;
    case QVariant::ULongLong:  lua_pushinteger(L, V->toULongLong());  break;
    case QVariant::Double:     lua_pushnumber (L, V->toDouble());     break;
    case QVariant::String:     lua_pushstring (L, (const char*)V->toString().toAscii().data());    break;
    case QVariant::KeySequence:     lua_pushstring (L, (const char*)V->toString().toAscii().data());    break;
    case QVariant::Invalid:
      lua_pushnil(L);
      lua_pushstring(L, "invalid value can't be pushed");
      return 2;
      break;
    default:
      if (V->canConvert(QVariant::Bool)) {
        lua_pushboolean(L, (int)V->toBool());
      } else if (V->canConvert(QVariant::Double)) {
        lua_pushnumber (L, V->toDouble());
      } else if (V->canConvert(QVariant::Int)) {
        lua_pushinteger(L, V->toInt());
      } else if (V->canConvert(QVariant::String)) {
        lua_pushstring (L, (const char*)V->toString().toAscii().data());
      } else {
//             lua_getfield(L, LUA_REGISTRYINDEX, V->typeName()); ...
        lua_pushnil(L);
        lua_pushstring(L, "QVariant type ");
        lua_pushstring(L, V->typeName());
        lua_pushstring(L, " can't be pushed");
        lua_concat(L, 3);
        return 2;
      }
      break;
  };
  return 1;
}





