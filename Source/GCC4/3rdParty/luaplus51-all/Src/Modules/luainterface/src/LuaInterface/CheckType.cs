using System;
using System.Reflection;

namespace LuaInterface
{
	/*
	 * Type checking and conversion functions.
	 * 
	 * Author: Fabio Mascarenhas
	 * Version: 1.0
	 */
	class CheckType
	{
		private ObjectTranslator translator;

		ExtractValue extractSbyte;
		ExtractValue extractByte;
		ExtractValue extractShort;
		ExtractValue extractUshort;
		ExtractValue extractInt;
		ExtractValue extractUint;
		ExtractValue extractLong;
		ExtractValue extractUlong;
		ExtractValue extractDouble;
		ExtractValue extractChar;
		ExtractValue extractFloat;
		ExtractValue extractDecimal;
		ExtractValue extractBoolean;
		ExtractValue extractString;
		ExtractValue extractFunction;
		ExtractValue extractTable;
		ExtractValue extractUserdata;
		ExtractValue extractObject;
		ExtractValue extractNetObject;

		public CheckType(ObjectTranslator translator) 
		{
			this.translator=translator;
			extractSbyte=new ExtractValue(this.getAsSbyte);
			extractByte=new ExtractValue(this.getAsByte);
			extractShort=new ExtractValue(this.getAsShort);
			extractUshort=new ExtractValue(this.getAsUshort);
			extractInt=new ExtractValue(this.getAsInt);
			extractUint=new ExtractValue(this.getAsUint);
			extractLong=new ExtractValue(this.getAsLong);
			extractUlong=new ExtractValue(this.getAsUlong);
			extractDouble=new ExtractValue(this.getAsDouble);
			extractChar=new ExtractValue(this.getAsChar);
			extractFloat=new ExtractValue(this.getAsFloat);
			extractDecimal=new ExtractValue(this.getAsDecimal);
			extractBoolean=new ExtractValue(this.getAsBoolean);
			extractString=new ExtractValue(this.getAsString);
			extractFunction=new ExtractValue(this.getAsFunction);
			extractTable=new ExtractValue(this.getAsTable);
			extractUserdata=new ExtractValue(this.getAsUserdata);
			extractObject=new ExtractValue(this.getAsObject);
			extractNetObject=new ExtractValue(this.getAsNetObject);
		}

		/*
		 * Checks if the value at Lua stack index stackPos matches paramType,
		 * returning a conversion function if it does and null otherwise.
		 */
        internal ExtractValue getExtractor(IReflect paramType)
        {
            return getExtractor(paramType.UnderlyingSystemType);
        }
		internal ExtractValue getExtractor(Type paramType) 
		{
			if(paramType.IsByRef) paramType=paramType.GetElementType();
			if(paramType.Equals(typeof(object))) 
				return extractObject;
			else if(paramType.Equals(typeof(sbyte))) 
				return extractSbyte;
			else if(paramType.Equals(typeof(byte)))
				return extractByte;
			else if(paramType.Equals(typeof(short)))
				return extractShort;
			else if(paramType.Equals(typeof(ushort)))
				return extractUshort;
			else if(paramType.Equals(typeof(int)))
				return extractInt;
			else if(paramType.Equals(typeof(uint)))
				return extractUint;
			else if(paramType.Equals(typeof(long)))
				return extractLong;
			else if(paramType.Equals(typeof(ulong)))
				return extractUlong;
			else if(paramType.Equals(typeof(char)))
				return extractChar;
			else if(paramType.Equals(typeof(float)))
				return extractFloat;
			else if(paramType.Equals(typeof(decimal)))
				return extractDecimal;
			else if(paramType.Equals(typeof(double)))
				return extractDouble;
			else if(paramType.Equals(typeof(bool))) 
				return extractBoolean;
			else if(paramType.Equals(typeof(string))) 
				return extractString;
			else if(paramType.Equals(typeof(LuaTable))) 
				return extractTable;
			else if(paramType.Equals(typeof(LuaUserData))) 
				return extractUserdata;
			else if(paramType.Equals(typeof(LuaFunction))) 
				return extractFunction;
			else
				return extractNetObject;
		}

		internal ExtractValue checkType(IntPtr luaState,int stackPos,Type paramType) 
		{
            LuaTypes luatype = LuaDLL.lua_type(luaState, stackPos);

			if(paramType.IsByRef) paramType=paramType.GetElementType();

            Type underlyingType = Nullable.GetUnderlyingType(paramType);
            if (underlyingType != null)
            {
                paramType = underlyingType;     // Silently convert nullable types to their non null requics
            }

			if(paramType.Equals(typeof(object))) 
			{
				return extractObject;
			}
			else if(paramType.Equals(typeof(sbyte))) 
			{
				if(LuaDLL.lua_isnumber(luaState,stackPos))
					return extractSbyte;
			}
			else if(paramType.Equals(typeof(byte)))
			{
				if(LuaDLL.lua_isnumber(luaState,stackPos))
					return extractByte;
			}
			else if(paramType.Equals(typeof(short)))
			{
				if(LuaDLL.lua_isnumber(luaState,stackPos))
					return extractShort;
			}
			else if(paramType.Equals(typeof(ushort)))
			{
				if(LuaDLL.lua_isnumber(luaState,stackPos))
					return extractUshort;
			}
			else if(paramType.Equals(typeof(int)))
			{
				if(LuaDLL.lua_isnumber(luaState,stackPos))
					return extractInt;
			}
			else if(paramType.Equals(typeof(uint)))
			{
				if(LuaDLL.lua_isnumber(luaState,stackPos))
					return extractUint;
			}
			else if(paramType.Equals(typeof(long)))
			{
				if(LuaDLL.lua_isnumber(luaState,stackPos))
					return extractLong;
			}
			else if(paramType.Equals(typeof(ulong)))
			{
				if(LuaDLL.lua_isnumber(luaState,stackPos))
					return extractUlong;
			}
			else if(paramType.Equals(typeof(char)))
			{
				if(LuaDLL.lua_isnumber(luaState,stackPos))
					return extractChar;
			}
			else if(paramType.Equals(typeof(float)))
			{
				if(LuaDLL.lua_isnumber(luaState,stackPos))
					return extractFloat;
			}
			else if(paramType.Equals(typeof(decimal)))
			{
				if(LuaDLL.lua_isnumber(luaState,stackPos))
					return extractDecimal;
			}
			else if(paramType.Equals(typeof(double)))
			{
				if(LuaDLL.lua_isnumber(luaState,stackPos))
					return extractDouble;
			}
			else if(paramType.Equals(typeof(bool))) 
			{
				if(LuaDLL.lua_isboolean(luaState,stackPos))
					return extractBoolean;
			}
			else if(paramType.Equals(typeof(string))) 
			{
                if (LuaDLL.lua_isstring(luaState, stackPos))
                    return extractString;
                else if (luatype == LuaTypes.LUA_TNIL)
                    return extractNetObject;            // kevinh - silently convert nil to a null string pointer
			}
			else if(paramType.Equals(typeof(LuaTable))) 
			{
                if (luatype == LuaTypes.LUA_TTABLE)
					return extractTable;
			}
			else if(paramType.Equals(typeof(LuaUserData))) 
			{
                if (luatype == LuaTypes.LUA_TUSERDATA)
					return extractUserdata;
			}
			else if(paramType.Equals(typeof(LuaFunction))) 
			{
                if (luatype == LuaTypes.LUA_TFUNCTION)
					return extractFunction;
			}
			else if(typeof(Delegate).IsAssignableFrom(paramType) &&
                luatype == LuaTypes.LUA_TFUNCTION) 
			{
				return new ExtractValue(new DelegateGenerator(translator,paramType).extractGenerated);
			}
			else if(paramType.IsInterface &&
                luatype == LuaTypes.LUA_TTABLE)
			{
				return new ExtractValue(new ClassGenerator(translator,paramType).extractGenerated);
			}
            else if ((paramType.IsInterface || paramType.IsClass) &&
                luatype == LuaTypes.LUA_TNIL)
            {
                // kevinh - allow nil to be silently converted to null - extractNetObject will return null when the item ain't found
                return extractNetObject;
            }
            else if (LuaDLL.lua_type(luaState, stackPos) == LuaTypes.LUA_TTABLE)
            {
                if (LuaDLL.luaL_getmetafield(luaState, stackPos, "__index"))
                {
                    object obj = translator.getNetObject(luaState, -1);
                    LuaDLL.lua_settop(luaState, -2);
                    if (obj != null && paramType.IsAssignableFrom(obj.GetType()))
                        return extractNetObject;
                }
                else return null;
            }
            else
            {
                object obj = translator.getNetObject(luaState, stackPos);
                if (obj != null && paramType.IsAssignableFrom(obj.GetType()))
                    return extractNetObject;
            }
			return null;
		}

		/*
		 * The following functions return the value in the Lua stack
		 * index stackPos as the desired type if it can, or null
		 * otherwise.
		 */
		private object getAsSbyte(IntPtr luaState,int stackPos) 
		{
			sbyte retVal=(sbyte)LuaDLL.lua_tonumber(luaState,stackPos);
			if(retVal==0 && !LuaDLL.lua_isnumber(luaState,stackPos)) return null;
			return retVal;
		}
		private object getAsByte(IntPtr luaState,int stackPos) 
		{
			byte retVal=(byte)LuaDLL.lua_tonumber(luaState,stackPos);
			if(retVal==0 && !LuaDLL.lua_isnumber(luaState,stackPos)) return null;
			return retVal;
		}
		private object getAsShort(IntPtr luaState,int stackPos) 
		{
			short retVal=(short)LuaDLL.lua_tonumber(luaState,stackPos);
			if(retVal==0 && !LuaDLL.lua_isnumber(luaState,stackPos)) return null;
			return retVal;
		}
		private object getAsUshort(IntPtr luaState,int stackPos) 
		{
			ushort retVal=(ushort)LuaDLL.lua_tonumber(luaState,stackPos);
			if(retVal==0 && !LuaDLL.lua_isnumber(luaState,stackPos)) return null;
			return retVal;
		}
		private object getAsInt(IntPtr luaState,int stackPos) 
		{
			int retVal=(int)LuaDLL.lua_tonumber(luaState,stackPos);
			if(retVal==0 && !LuaDLL.lua_isnumber(luaState,stackPos)) return null;
			return retVal;
		}
		private object getAsUint(IntPtr luaState,int stackPos) 
		{
			uint retVal=(uint)LuaDLL.lua_tonumber(luaState,stackPos);
			if(retVal==0 && !LuaDLL.lua_isnumber(luaState,stackPos)) return null;
			return retVal;
		}
		private object getAsLong(IntPtr luaState,int stackPos) 
		{
			long retVal=(long)LuaDLL.lua_tonumber(luaState,stackPos);
			if(retVal==0 && !LuaDLL.lua_isnumber(luaState,stackPos)) return null;
			return retVal;
		}
		private object getAsUlong(IntPtr luaState,int stackPos) 
		{
			ulong retVal=(ulong)LuaDLL.lua_tonumber(luaState,stackPos);
			if(retVal==0 && !LuaDLL.lua_isnumber(luaState,stackPos)) return null;
			return retVal;
		}
		private object getAsDouble(IntPtr luaState,int stackPos) 
		{
			double retVal=LuaDLL.lua_tonumber(luaState,stackPos);
			if(retVal==0 && !LuaDLL.lua_isnumber(luaState,stackPos)) return null;
			return retVal;
		}
		private object getAsChar(IntPtr luaState,int stackPos) 
		{
			char retVal=(char)LuaDLL.lua_tonumber(luaState,stackPos);
			if(retVal==0 && !LuaDLL.lua_isnumber(luaState,stackPos)) return null;
			return retVal;
		}
		private object getAsFloat(IntPtr luaState,int stackPos) 
		{
			float retVal=(float)LuaDLL.lua_tonumber(luaState,stackPos);
			if(retVal==0 && !LuaDLL.lua_isnumber(luaState,stackPos)) return null;
			return retVal;
		}
		private object getAsDecimal(IntPtr luaState,int stackPos) 
		{
			decimal retVal=(decimal)LuaDLL.lua_tonumber(luaState,stackPos);
			if(retVal==0 && !LuaDLL.lua_isnumber(luaState,stackPos)) return null;
			return retVal;
		}
		private object getAsBoolean(IntPtr luaState,int stackPos) 
		{
			return LuaDLL.lua_toboolean(luaState,stackPos);
		}
		private object getAsString(IntPtr luaState,int stackPos) 
		{
			string retVal=LuaDLL.lua_tostring(luaState,stackPos);
			if(retVal=="" && !LuaDLL.lua_isstring(luaState,stackPos)) return null;
			return retVal;
		}
		private object getAsTable(IntPtr luaState,int stackPos) 
		{
			return translator.getTable(luaState,stackPos);
		}
		private object getAsFunction(IntPtr luaState,int stackPos) 
		{
			return translator.getFunction(luaState,stackPos);
		}
		private object getAsUserdata(IntPtr luaState,int stackPos) 
		{
			return translator.getUserData(luaState,stackPos);
		}
		public object getAsObject(IntPtr luaState,int stackPos) 
		{
			if(LuaDLL.lua_type(luaState,stackPos)==LuaTypes.LUA_TTABLE) 
			{
				if(LuaDLL.luaL_getmetafield(luaState,stackPos,"__index")) 
				{
					if(LuaDLL.luaL_checkmetatable(luaState,-1)) 
					{
						LuaDLL.lua_insert(luaState,stackPos);
						LuaDLL.lua_remove(luaState,stackPos+1);
					} 
					else 
					{
						LuaDLL.lua_settop(luaState,-2);
					}
				}
			}
			object obj=translator.getObject(luaState,stackPos);
			return obj;
		}
		public object getAsNetObject(IntPtr luaState,int stackPos) 
		{
			object obj=translator.getNetObject(luaState,stackPos);
			if(obj==null && LuaDLL.lua_type(luaState,stackPos)==LuaTypes.LUA_TTABLE) 
			{
				if(LuaDLL.luaL_getmetafield(luaState,stackPos,"__index")) 
				{
					if(LuaDLL.luaL_checkmetatable(luaState,-1)) 
					{
						LuaDLL.lua_insert(luaState,stackPos);
						LuaDLL.lua_remove(luaState,stackPos+1);
						obj=translator.getNetObject(luaState,stackPos);
					} 
					else 
					{
						LuaDLL.lua_settop(luaState,-2);
					}
				}
			}
			return obj;
		}
	}
}
