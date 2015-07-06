#OLD: !IF $(LUA_VERSION) == 5
LUACOM_LUA = $(SRC_LIB_DIR)\luacom5.loh
$(LUACOM_LUA): $(SRC_LIB_DIR)\luacom5.lua
#OLD: !ENDIF

$(OBJ_DIR)/LuaAux.obj : $(SRC_LIB_DIR)/LuaAux.cpp $(SRC_LIB_DIR)/LuaAux.h $(SRC_LIB_DIR)/LuaCompat.h

$(OBJ_DIR)/luabeans.obj : $(SRC_LIB_DIR)/luabeans.cpp $(SRC_LIB_DIR)/luabeans.h $(SRC_LIB_DIR)/LuaCompat.h $(SRC_LIB_DIR)/tLuaCOMException.h $(SRC_LIB_DIR)/LuaAux.h $(SRC_LIB_DIR)/tUtil.h

$(OBJ_DIR)/tLuaCOM.obj : $(SRC_LIB_DIR)/tLuaCOM.cpp $(SRC_LIB_DIR)/tLuaCOM.h $(SRC_LIB_DIR)/tLuaDispatch.h $(SRC_LIB_DIR)/LuaAux.h $(SRC_LIB_DIR)/tUtil.h $(SRC_LIB_DIR)/tLuaCOMException.h $(SRC_LIB_DIR)/tCOMUtil.h $(SRC_LIB_DIR)/LuaCompat.h

$(OBJ_DIR)/tLuaCOMException.obj : $(SRC_LIB_DIR)/tLuaCOMException.cpp $(SRC_LIB_DIR)/tLuaCOMException.h $(SRC_LIB_DIR)/tUtil.h

$(OBJ_DIR)/tLuaCOMTypeHandler.obj : $(SRC_LIB_DIR)/tLuaCOMTypeHandler.cpp $(SRC_LIB_DIR)/tLuaCOMTypeHandler.h $(SRC_LIB_DIR)/tLuaCOM.h $(SRC_LIB_DIR)/tLuaVector.h $(SRC_LIB_DIR)/tLuaCOMException.h $(SRC_LIB_DIR)/tUtil.h $(SRC_LIB_DIR)/LuaAux.h $(SRC_LIB_DIR)/LuaCompat.h $(SRC_LIB_DIR)/tLuaCOMEnumerator.h $(SRC_LIB_DIR)/tCOMUtil.h $(SRC_LIB_DIR)/luabeans.h $(SRC_LIB_DIR)/luacom_internal.h

$(OBJ_DIR)/tLuaDispatch.obj : $(SRC_LIB_DIR)/tLuaDispatch.cpp $(SRC_LIB_DIR)/tLuaDispatch.h $(SRC_LIB_DIR)/tLuaCOM.h $(SRC_LIB_DIR)/tLuaCOMException.h $(SRC_LIB_DIR)/tCOMUtil.h $(SRC_LIB_DIR)/LuaAux.h $(SRC_LIB_DIR)/tUtil.h $(SRC_LIB_DIR)/LuaCompat.h

$(OBJ_DIR)/tLuaObjList.obj : $(SRC_LIB_DIR)/tLuaObjList.cpp $(SRC_LIB_DIR)/tLuaObjList.h $(SRC_LIB_DIR)/tLuaCOMException.h

$(OBJ_DIR)/tLuaVector.obj : $(SRC_LIB_DIR)/tLuaVector.cpp $(SRC_LIB_DIR)/tLuaVector.h $(SRC_LIB_DIR)/tLuaCOM.h $(SRC_LIB_DIR)/LuaCompat.h $(SRC_LIB_DIR)/LuaAux.h $(SRC_LIB_DIR)/tLuaCOMException.h

$(OBJ_DIR)/tStringBuffer.obj : $(SRC_LIB_DIR)/tStringBuffer.cpp $(SRC_LIB_DIR)/tStringBuffer.h

$(OBJ_DIR)/tUtil.obj : $(SRC_LIB_DIR)/tUtil.cpp $(SRC_LIB_DIR)/tUtil.h $(SRC_LIB_DIR)/tLuaCOMException.h

$(OBJ_DIR)/tCOMUtil.obj : $(SRC_LIB_DIR)/tCOMUtil.cpp $(SRC_LIB_DIR)/tCOMUtil.h

$(OBJ_DIR)/tLuaCOMClassFactory.obj : $(SRC_LIB_DIR)/tLuaCOMClassFactory.cpp $(SRC_LIB_DIR)/tLuaCOMClassFactory.h $(SRC_LIB_DIR)/LuaCompat.h $(SRC_LIB_DIR)/tLuaCOMException.h $(SRC_LIB_DIR)/tUtil.h $(INC_DIR)/luacom.h

$(OBJ_DIR)/luacom_console.obj : $(SRC_BIN_DIR)/luacom_console.cpp $(INC_DIR)/luacom.h

$(OBJ_DIR)/tLuaCOMConnPoints.obj : $(SRC_LIB_DIR)/tLuaCOMConnPoints.cpp $(SRC_LIB_DIR)/tLuaCOMConnPoints.h $(SRC_LIB_DIR)/tLuaCOM.h $(SRC_LIB_DIR)/tCOMUtil.h $(SRC_LIB_DIR)/tLuaCOMException.h $(SRC_LIB_DIR)/LuaCompat.h $(SRC_LIB_DIR)/LuaAux.h $(SRC_LIB_DIR)/luacom_internal.h

$(OBJ_DIR)/LuaCompat.obj : $(SRC_LIB_DIR)/LuaCompat.c $(SRC_LIB_DIR)/LuaCompat.h

$(OBJ_DIR)/tLuaCOMEnumerator.obj : $(SRC_LIB_DIR)/tLuaCOMEnumerator.cpp $(SRC_LIB_DIR)/tLuaCOMEnumerator.h $(SRC_LIB_DIR)/tLuaCOMTypeHandler.h $(SRC_LIB_DIR)/luabeans.h $(SRC_LIB_DIR)/tCOMUtil.h $(SRC_LIB_DIR)/LuaAux.h $(SRC_LIB_DIR)/tUtil.h $(SRC_LIB_DIR)/tLuaCOMException.h $(SRC_LIB_DIR)/luacom_internal.h $(SRC_LIB_DIR)/luacompat.h

$(OBJ_DIR)/tLuaObject.obj : $(SRC_LIB_DIR)/tLuaObject.cpp $(SRC_LIB_DIR)/tLuaObject.h $(SRC_LIB_DIR)/LuaAux.h $(SRC_LIB_DIR)/tLuaCOMException.h $(SRC_LIB_DIR)/tUtil.h $(SRC_LIB_DIR)/luacom_internal.h $(SRC_LIB_DIR)/LuaCompat.h

$(OBJ_DIR)/tLuaControl.obj : $(SRC_LIB_DIR)/tLuaControl.cpp $(SRC_LIB_DIR)/tLuaControl.h $(SRC_LIB_DIR)/tLuaDispatch.h $(SRC_LIB_DIR)/luacom_internal.h $(SRC_LIB_DIR)/LuaCompat.h

$(OBJ_DIR)/tLuaTLB.obj : $(SRC_LIB_DIR)/tLuaTLB.cpp $(SRC_LIB_DIR)/tLuaTLB.h $(SRC_LIB_DIR)/tCOMUtil.h $(SRC_LIB_DIR)/tLuaCOMException.h $(SRC_LIB_DIR)/tUtil.h $(SRC_LIB_DIR)/luacom_internal.h $(SRC_LIB_DIR)/LuaAux.h $(SRC_LIB_DIR)/LuaCompat.h $(SRC_LIB_DIR)/tLuaCOM.h $(SRC_LIB_DIR)/tLuaCOMTypeHandler.h

$(OBJ_DIR)/luacom.obj : $(SRC_LIB_DIR)/luacom.cpp $(INC_DIR)/luacom.h $(SRC_LIB_DIR)/luabeans.h $(SRC_LIB_DIR)/tLuaDispatch.h $(SRC_LIB_DIR)/tLuaCOM.h $(SRC_LIB_DIR)/tLuaCOMException.h $(SRC_LIB_DIR)/LuaAux.h $(SRC_LIB_DIR)/tUtil.h $(SRC_LIB_DIR)/LuaCompat.h $(SRC_LIB_DIR)/tLuaCOMEnumerator.h $(LUACOM_LUA) $(SRC_LIB_DIR)/tCOMUtil.h $(SRC_LIB_DIR)/tLuaCOMClassFactory.h $(SRC_LIB_DIR)/tLuaCOMTypeHandler.h $(SRC_LIB_DIR)/tLuaCOMConnPoints.h $(SRC_LIB_DIR)/luacom_internal.h $(SRC_LIB_DIR)/tLuaTLB.h

$(OBJ_DIR)/luacom_dll.obj : $(SRC_DLL_DIR)/luacom_dll.cpp $(SRC_DLL_DIR)/luacom_dll.h $(INC_DIR)/luacom.h $(SRC_LIB_DIR)/LuaCompat.h $(SRC_LIB_DIR)/luacom_internal.h $(SRC_LIB_DIR)/tLuaCOMClassFactory.h $(SRC_LIB_DIR)/tLuaCOMException.h $(SRC_LIB_DIR)/tUtil.h $(SRC_LIB_DIR)/tCOMUtil.h

