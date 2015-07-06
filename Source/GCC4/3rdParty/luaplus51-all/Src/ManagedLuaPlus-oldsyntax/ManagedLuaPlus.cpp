// This is the main DLL file.

#include "stdafx.h"

#include "ManagedLuaPlus.h"
#include <crtdbg.h>
#include <tchar.h>
#include <vcclr.h>
//#include "_vcclrit.h"
#undef LoadString

LUA_EXTERN_C_BEGIN
#include "LuaPlus/src/lua.h"
LUA_EXTERN_C_END

#undef GetErrorMessage
#pragma warning(disable: 4239)

//int main()
//{
//}

struct lua_State {};
struct lua_TValue {};

namespace ManagedLuaPlus {

using namespace System::Reflection;
using System::Runtime::InteropServices::GCHandle;
using namespace System::Collections;

public __gc class LuaState;
public __gc class LuaStackObject;
public __gc class LuaObject;
public __gc interface LuaStateOutFile;

public __delegate int LuaPlusCallback(LuaState* state);
public __delegate int LuaPlusOldCallback(LuaState* state, LuaStackObject* args[]);

class AString
{
public:
	AString(const AString& src) :
		 m_buffer(NULL)
	{
		Duplicate(src.m_buffer);
	}

	AString(const char* str) :
		m_buffer(NULL)
	{
		Duplicate(str);
	}

	~AString()
	{
		if (m_buffer)
			free(m_buffer);
	}

	operator const char*() const
	{
		return m_buffer;
	}

	AString& operator=(const AString& src)
	{
		Duplicate(src.m_buffer);
	}

protected:
	void Duplicate(const char* str)
	{
		if (m_buffer)
			free(m_buffer);
		m_buffer = (char*)malloc(strlen(str) + 1);
		strcpy(m_buffer, str);
	}

	char* m_buffer;
};



class WString
{
public:
	WString(const WString& src) :
		 m_buffer(NULL)
	{
		Duplicate(src.m_buffer);
	}

	WString(const wchar_t* str) :
		m_buffer(NULL)
	{
		Duplicate(str);
	}

	~WString()
	{
		delete[] m_buffer;
	}

	operator const wchar_t*() const
	{
		return m_buffer;
	}

	WString& operator=(const WString& src)
	{
		Duplicate(src.m_buffer);
	}

protected:
	void Duplicate(const wchar_t* str)
	{
		delete[] m_buffer;
		m_buffer = new wchar_t[wcslen(str) + 1];
		wcscpy(m_buffer, str);
	}

	wchar_t* m_buffer;
};



using namespace System::Runtime::InteropServices;

class MarshalStringANSI
{
public:
	MarshalStringANSI(System::String* s)
	{
		m_string = (const char*)(Marshal::StringToHGlobalAnsi(s)).ToPointer();
	}

	~MarshalStringANSI()
	{
		Marshal::FreeHGlobal(IntPtr((void*)m_string));
	}

	operator const char*() const
	{
		return m_string;
	}

protected:
	const char* m_string;
};


class MarshalStringUnicode
{
public:
	MarshalStringUnicode(System::String* s)
	{
		m_string = (const wchar_t*)(Marshal::StringToHGlobalUni(s)).ToPointer();
	}

	~MarshalStringUnicode()
	{
		Marshal::FreeHGlobal(IntPtr((void*)m_string));
	}

	operator const wchar_t*() const
	{
		return m_string;
	}

	operator const lua_WChar*() const
	{
		return (const lua_WChar*)(const wchar_t*)m_string;
	}

protected:
	const wchar_t* m_string;
};


public __gc class LuaNode
{
public:
	LuaNode(LuaObject* key, LuaObject* value) :
		m_key(key),
		m_value(value)
	{
	}

	__property LuaObject* get_Key()
	{
		return m_key;
	}

	__property LuaObject* get_Value()
	{
		return m_value;
	}

	String* ToString();

private:
	LuaObject* m_key;
	LuaObject* m_value;
};


public __gc class LuaException : public Exception
{
public:
	LuaException(String* message) :
		m_message(message)
	{
	}

	__property String* get_Message()
	{
		return m_message;
	}

private:
	String* m_message;
};


[System::Reflection::DefaultMemberAttribute(S"Item")]
public __gc class LuaObject : public IEnumerable
{
public:
	LuaObject();
	LuaObject(ManagedLuaPlus::LuaObject* object);
	LuaObject(ManagedLuaPlus::LuaState* state, LuaPlus::LuaObject& object);
	LuaObject(ManagedLuaPlus::LuaState* state);
	LuaObject(ManagedLuaPlus::LuaState* state, int stackIndex);
	LuaObject(ManagedLuaPlus::LuaStackObject* object);

	static LuaObject* op_Assign(LuaObject* left, LuaObject* right)
	{
		delete left->m_object;
		left->m_state = right->m_state;
		left->m_object = new LuaPlus::LuaObject(*right->m_object);
		return left;
	}

	~LuaObject();

	LuaState* GetState();

	static int op_Equality(LuaObject* left, LuaObject* right)
	{
		return *left->m_object == *right->m_object;
	}

	static int op_LessThan(LuaObject* left, LuaObject* right)
	{
		return *left->m_object < *right->m_object;
	}

	String* TypeName()
	{
		try
		{
			return new String(m_object->TypeName());
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	int Type()
	{
		try
		{
			return m_object->Type();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	bool IsNil()
	{
		try
		{
			return m_object->IsNil();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	bool IsTable()
	{
		try
		{
			return m_object->IsTable();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	bool IsUserData()
	{
		try
		{
			return m_object->IsUserData();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	bool IsCFunction()
	{
		try
		{
			return m_object->IsCFunction();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	bool IsInteger()
	{
		try
		{
			return m_object->IsInteger();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	bool IsNumber()
	{
		try
		{
			return m_object->IsNumber();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	bool IsString()
	{
		try
		{
			return m_object->IsString();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	bool IsWString()
	{
		try
		{
			return m_object->IsWString();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	bool IsConvertibleToInteger()
	{
		try
		{
			return m_object->IsConvertibleToInteger();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	bool IsConvertibleToNumber()
	{
		try
		{
			return m_object->IsConvertibleToNumber();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	bool IsConvertibleToString()
	{
		try
		{
			return m_object->IsConvertibleToString();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	bool IsConvertibleToWString()
	{
		try
		{
			return m_object->IsConvertibleToWString();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	bool IsFunction()
	{
		try
		{
			return m_object->IsFunction();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	bool IsNone()
	{
		try
		{
			return m_object->IsNone();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	bool IsLightUserData()
	{
		try
		{
			return m_object->IsLightUserData();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	bool IsBoolean()
	{
		try
		{
			return m_object->IsBoolean();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	int ToInteger()
	{
		try
		{
			return m_object->ToInteger();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	double ToNumber()
	{
		try
		{
			return m_object->ToNumber();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	String* ToString()
	{
		try
		{
			return new String(m_object->ToString());
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	String* ToWString()
	{
		try
		{
			return new String((const wchar_t*)m_object->ToWString());
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	size_t ToStrLen()
	{
		try
		{
			return m_object->ToStrLen();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	int GetInteger()
	{
		try
		{
			return m_object->GetInteger();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	float GetFloat()
	{
		try
		{
			return m_object->GetFloat();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	double GetNumber()
	{
		try
		{
			return m_object->GetNumber();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	String* GetString()
	{
		try
		{
			return new String(m_object->GetString());
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	String* GetWString()
	{
		try
		{
			return new String((const wchar_t*)m_object->GetWString());
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	int StrLen()
	{
		try
		{
			return m_object->StrLen();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	lua_CFunction GetCFunction()
	{
		try
		{
			return m_object->GetCFunction();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	void* GetUserData()
	{
		try
		{
			return m_object->GetUserData();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	const void* GetLuaPointer()
	{
		try
		{
			return m_object->GetLuaPointer();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	void* GetLightUserData()
	{
		try
		{
			return m_object->GetLightUserData();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	bool GetBoolean()
	{
		try
		{
			return m_object->GetBoolean();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	LuaStackObject* Push();

//	LuaStackObject __cdecl PCall(const char* argTypes, ...);
//	LuaStackObject __cdecl PCall(int nresults, const char* argTypes, ...);

	LuaObject* GetMetaTable()
	{
		try
		{
			return new LuaObject(GetState(), m_object->GetMetaTable());
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	void SetMetaTable(LuaObject* value)
	{
		try
		{
			m_object->SetMetaTable(*value->m_object);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	void Insert(LuaObject* obj)
	{
		try
		{
			m_object->Insert(*obj->m_object);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	void Insert(int index, LuaObject* obj)
	{
		try
		{
			m_object->Insert(index, *obj->m_object);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	void Remove(int index)
	{
		try
		{
			m_object->Remove(index);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	void Sort()
	{
		try
		{
			m_object->Sort();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	int GetCount()
	{
		try
		{
			return m_object->GetCount();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	int GetTableCount()
	{
		try
		{
			return m_object->GetTableCount();
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	LuaObject* Clone()
	{
		try
		{
			return new LuaObject(GetState(), m_object->Clone());
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	void DeepClone(LuaObject* outObj)
	{
		try
		{
			m_object->DeepClone(*outObj->m_object);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	LuaObject* CreateTable(String* key)
	{
		try
		{
			return new LuaObject(GetState(), m_object->CreateTable(MarshalStringANSI(key)));
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	LuaObject* CreateTable(int key)
	{
		try
		{
			return new LuaObject(GetState(), m_object->CreateTable(key));
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	LuaObject* CreateTable(LuaObject* key)
	{
		try
		{
			return new LuaObject(GetState(), m_object->CreateTable(*key->m_object));
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// Metatable-capable sets
	LuaObject* SetNil(String* key)
	{
		try
		{
			m_object->SetNil(MarshalStringANSI(key));
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetNil(int key)
	{
		try
		{
			m_object->SetNil(key);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetNil(LuaObject* key)
	{
		try
		{
			m_object->SetNil(*key->m_object);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetBoolean(String* key, bool value)
	{
		try
		{
			m_object->SetBoolean(MarshalStringANSI(key), value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetBoolean(int key, bool value)
	{
		try
		{
			m_object->SetBoolean(key, value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetBoolean(LuaObject* key, bool value)
	{
		try
		{
			m_object->SetBoolean(*key->m_object, value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetInteger(String* key, int value)
	{
		try
		{
			m_object->SetInteger(MarshalStringANSI(key), value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetInteger(int key, int value)
	{
		try
		{
			m_object->SetInteger(key, value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetInteger(LuaObject* key, int value)
	{
		try
		{
			m_object->SetInteger(*key->m_object, value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetNumber(String* key, lua_Number value)
	{
		try
		{
			m_object->SetNumber(MarshalStringANSI(key), value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetNumber(int key, lua_Number value)
	{
		try
		{
			m_object->SetNumber(key, value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetNumber(LuaObject* key, lua_Number value)
	{
		try
		{
			m_object->SetNumber(*key->m_object, value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetString(String* key, String* value)
	{
		try
		{
			m_object->SetString(MarshalStringANSI(key), MarshalStringANSI(value));
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetString(int key, String* value)
	{
		try
		{
			m_object->SetString(key, MarshalStringANSI(value));
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetString(LuaObject* key, String* value)
	{
		try
		{
			m_object->SetString(*key->m_object, MarshalStringANSI(value));
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetWString(String* key, String* value)
	{
		try
		{
			m_object->SetWString(MarshalStringANSI(key), MarshalStringUnicode(value));
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetWString(int key, String* value)
	{
		try
		{
			m_object->SetWString(key, MarshalStringUnicode(value));
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetWString(LuaObject* key, String* value)
	{
		try
		{
			m_object->SetWString(*key->m_object, MarshalStringUnicode(value));
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetString(String* key, String* value, unsigned int len)
	{
		try
		{
			m_object->SetString(MarshalStringANSI(key), MarshalStringANSI(value), len);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetString(int key, String* value, unsigned int len)
	{
		try
		{
			m_object->SetString(key, MarshalStringANSI(value), len);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetString(LuaObject* key, String* value, unsigned int len)
	{
		try
		{
			m_object->SetString(*key->m_object, MarshalStringANSI(value), len);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetWString(String* key, String* value, unsigned int len)
	{
		try
		{
			m_object->SetWString(MarshalStringANSI(key), MarshalStringUnicode(value), len);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetWString(int key, String* value, unsigned int len)
	{
		try
		{
			m_object->SetWString(key, MarshalStringUnicode(value), len);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetWString(LuaObject* key, String* value, unsigned int len)
	{
		try
		{
			m_object->SetWString(*key->m_object, MarshalStringUnicode(value), len);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetUserData(String* key, void* value)
	{
		try
		{
			m_object->SetUserData(MarshalStringANSI(key), value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetUserData(int key, void* value)
	{
		try
		{
			m_object->SetUserData(key, value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetUserData(LuaObject* key, void* value)
	{
		try
		{
			m_object->SetUserData(*key->m_object, value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetLightUserData(String* key, void* value)
	{
		try
		{
			m_object->SetLightUserData(MarshalStringANSI(key), value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetLightUserData(int key, void* value)
	{
		try
		{
			m_object->SetLightUserData(key, value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetLightUserData(LuaObject* key, void* value)
	{
		try
		{
			m_object->SetLightUserData(*key->m_object, value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetObject(String* key, LuaObject* value)
	{
		try
		{
			m_object->SetObject(MarshalStringANSI(key), *value->m_object);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetObject(int key, LuaObject* value)
	{
		try
		{
			m_object->SetObject(key, *value->m_object);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* SetObject(LuaObject* key, LuaObject* value)
	{
		try
		{
			m_object->SetObject(*key->m_object, *value->m_object);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}


	///////////////////////////////////////////////////////////////////////////
	// Raw sets
	LuaObject* RawSetNil(String* key)
	{
		try
		{
			m_object->RawSetNil(MarshalStringANSI(key));
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetNil(int key)
	{
		try
		{
			m_object->RawSetNil(key);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetNil(LuaObject* key)
	{
		try
		{
			m_object->RawSetNil(*key->m_object);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetBoolean(String* key, bool value)
	{
		try
		{
			m_object->RawSetBoolean(MarshalStringANSI(key), value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetBoolean(int key, bool value)
	{
		try
		{
			m_object->RawSetBoolean(key, value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetBoolean(LuaObject* key, bool value)
	{
		try
		{
			m_object->RawSetBoolean(*key->m_object, value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetInteger(String* key, int value)
	{
		try
		{
			m_object->RawSetInteger(MarshalStringANSI(key), value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetInteger(int key, int value)
	{
		try
		{
			m_object->RawSetInteger(key, value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetInteger(LuaObject* key, int value)
	{
		try
		{
			m_object->RawSetInteger(*key->m_object, value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetNumber(String* key, lua_Number value)
	{
		try
		{
			m_object->RawSetNumber(MarshalStringANSI(key), value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetNumber(int key, lua_Number value)
	{
		try
		{
			m_object->RawSetNumber(key, value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetNumber(LuaObject* key, lua_Number value)
	{
		try
		{
			m_object->RawSetNumber(*key->m_object, value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetString(String* key, String* value)
	{
		try
		{
			m_object->RawSetString(MarshalStringANSI(key), MarshalStringANSI(value));
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetString(int key, String* value)
	{
		try
		{
			m_object->RawSetString(key, MarshalStringANSI(value));
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetString(LuaObject* key, String* value)
	{
		try
		{
			m_object->RawSetString(*key->m_object, MarshalStringANSI(value));
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetWString(String* key, String* value)
	{
		try
		{
			m_object->RawSetWString(MarshalStringANSI(key), MarshalStringUnicode(value));
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetWString(int key, String* value)
	{
		try
		{
			m_object->RawSetWString(key, MarshalStringUnicode(value));
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetWString(LuaObject* key, String* value)
	{
		try
		{
			m_object->RawSetWString(*key->m_object, MarshalStringUnicode(value));
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetString(String* key, String* value, unsigned int len)
	{
		try
		{
			m_object->RawSetString(MarshalStringANSI(key), MarshalStringANSI(value), len);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetString(int key, String* value, unsigned int len)
	{
		try
		{
			m_object->RawSetString(key, MarshalStringANSI(value), len);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetString(LuaObject* key, String* value, unsigned int len)
	{
		try
		{
			m_object->RawSetString(*key->m_object, MarshalStringANSI(value), len);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetWString(String* key, String* value, unsigned int len)
	{
		try
		{
			m_object->RawSetWString(MarshalStringANSI(key), MarshalStringUnicode(value), len);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetWString(int key, String* value, unsigned int len)
	{
		try
		{
			m_object->RawSetWString(key, MarshalStringUnicode(value), len);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetWString(LuaObject* key, String* value, unsigned int len)
	{
		try
		{
			m_object->RawSetWString(*key->m_object, MarshalStringUnicode(value), len);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetUserData(String* key, void* value)
	{
		try
		{
			m_object->RawSetUserData(MarshalStringANSI(key), value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetUserData(int key, void* value)
	{
		try
		{
			m_object->RawSetUserData(key, value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetUserData(LuaObject* key, void* value)
	{
		try
		{
			m_object->RawSetUserData(*key->m_object, value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetLightUserData(String* key, void* value)
	{
		try
		{
			m_object->RawSetLightUserData(MarshalStringANSI(key), value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetLightUserData(int key, void* value)
	{
		try
		{
			m_object->RawSetLightUserData(key, value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetLightUserData(LuaObject* key, void* value)
	{
		try
		{
			m_object->RawSetLightUserData(*key->m_object, value);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetObject(String* key, LuaObject* value)
	{
		try
		{
			m_object->RawSetObject(MarshalStringANSI(key), *value->m_object);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetObject(int key, LuaObject* value)
	{
		try
		{
			m_object->RawSetObject(key, *value->m_object);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}

	LuaObject* RawSetObject(LuaObject* key, LuaObject* value)
	{
		try
		{
			m_object->RawSetObject(*key->m_object, *value->m_object);
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}

		return this;
	}


	void AssignNil(ManagedLuaPlus::LuaState* state);
	void AssignBoolean(ManagedLuaPlus::LuaState* state, bool value);
	void AssignNumber(ManagedLuaPlus::LuaState* state, lua_Number value);
	void AssignString(ManagedLuaPlus::LuaState* state, String* value);
	void AssignString(ManagedLuaPlus::LuaState* state, String* value, unsigned int len);
	void AssignWString(ManagedLuaPlus::LuaState* state, String* value);
	void AssignWString(ManagedLuaPlus::LuaState* state, String* value, unsigned int len);
	void AssignUserData(ManagedLuaPlus::LuaState* state, void* value);
	void AssignLightUserData(ManagedLuaPlus::LuaState* state, void* value);
	void AssignObject(LuaObject* value);
	void AssignNewTable(ManagedLuaPlus::LuaState* state);

	LuaObject* GetByName(String* name)
	{
		try
		{
			return new LuaObject(GetState(), m_object->GetByName(MarshalStringANSI(name)));
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	LuaObject* GetByIndex(int index)
	{
		try
		{
			return new LuaObject(GetState(), m_object->GetByIndex(index));
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	LuaObject* GetByObject(LuaStackObject* obj);

	LuaObject* GetByObject(LuaObject* obj)
	{
		try
		{
			return new LuaObject(GetState(), m_object->GetByObject(*obj->m_object));
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	LuaObject* RawGetByName(String* name)
	{
		try
		{
			return new LuaObject(GetState(), m_object->RawGetByName(MarshalStringANSI(name)));
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	LuaObject* RawGetByIndex(int index)
	{
		try
		{
			return new LuaObject(GetState(), m_object->RawGetByIndex(index));
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	LuaObject* RawGetByObject(LuaStackObject* obj);

	LuaObject* RawGetByObject(LuaObject* obj)
	{
		try
		{
			return new LuaObject(GetState(), m_object->RawGetByObject(*obj->m_object));
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	__property LuaObject* get_Item(String* name)
	{
		try
		{
			return new LuaObject(GetState(), m_object->RawGetByName(MarshalStringANSI(name)));
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	__property LuaObject* get_Item(int index)
	{
		try
		{
			return new LuaObject(GetState(), m_object->RawGetByIndex(index));
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	__property LuaObject* get_Item(LuaStackObject* obj);

	__property LuaObject* get_Item(LuaObject* obj)
	{
		try
		{
			return new LuaObject(GetState(), m_object->RawGetByObject(*obj->m_object));
		}
		catch (const LuaPlus::LuaException& e)
		{
			throw new LuaException(new String(e.GetErrorMessage()));
		}
	}

	LuaObject* Lookup(String* key)
	{
		return new LuaObject(GetState(), m_object->Lookup(MarshalStringANSI(key)));
	}

	void Register(String* funcName, LuaPlusCallback* function);
	void Register(String* funcName, LuaPlusOldCallback* function);
	void Register(String* funcName, Object* instance, MethodInfo* function);
	void Register(String* funcName, Delegate* function);

	void Call([ParamArray] Object* objs[]);

	__property String* get_WatchType()
	{
		int type = Type();

		switch (type)
		{
			case LUA_TNONE:
				return new String("NONE");

			case LUA_TNIL:
				return new String("NIL");

			case LUA_TBOOLEAN:
			{
				String* str = String::Format("BOOLEAN: {0}", GetBoolean() ? S"true" : S"false");
				return str;
			}

			case LUA_TLIGHTUSERDATA:
			{
				String* str = String::Format("LIGHTUSERDATA: {0:X}", __box((size_t)GetLightUserData()));
				return str;
			}

			case LUA_TNUMBER:
			{
				String* str = String::Format("NUMBER: {0:G}", __box(GetNumber()));
				return str;
			}

			case LUA_TSTRING:
			{
				String* str = String::Format("STRING: {0}", GetString());
				return str;
			}

			case LUA_TTABLE:
			{
				String* str = new String("TABLE");
				return str;
			}

			case LUA_TFUNCTION:
			{
				String* str = new String("FUNCTION");
				return str;
			}

			case LUA_TUSERDATA:
			{
				String* str = String::Format("USERDATA: {0:X}", __box((size_t)GetUserData()));
				return str;
			}

			case LUA_TTHREAD:
			{
				String* str = new String("FORMAT");
				return str;
			}

			case LUA_TWSTRING:
			{
				String* str = String::Format(L"WSTRING: {0}", GetWString());
				return str;
			}
		}

		return new String("Unknown");
	}

	__gc class TableEnumerator : public IEnumerator, public IDisposable
	{
	public:
		TableEnumerator(LuaObject* obj)
		{
			m_table = obj;
			if (!m_table->IsTable())
				throw new LuaException(new String("Object must be a table to enumerate over"));
			m_it = new LuaPlus::LuaTableIterator(*obj->m_object, true);
			m_atBeginning = true;
		}

		~TableEnumerator()
		{
			delete m_it;
		}

		void Reset()
		{
			m_it->Reset();
			m_atBeginning = true;
		}

		bool MoveNext()
		{
			if (!m_atBeginning)
			{
				return m_it->Next();
			}

			m_atBeginning = false;
			return m_it->IsValid();
		}

		__property Object* get_Current()
		{
			return new LuaNode(new LuaObject(m_table->GetState(), m_it->GetKey()), new LuaObject(m_table->GetState(), m_it->GetValue()));
		}

		void Dispose()
		{
			delete m_it;
			m_it = NULL;
			m_table = NULL;
			GC::SuppressFinalize(this);
		}

	private:
		LuaPlus::LuaTableIterator __nogc* m_it;
		LuaObject* m_table;
		bool m_atBeginning;
	};

	IEnumerator* GetEnumerator()
	{
		return new TableEnumerator(this);
	}

	// Implement the GetEnumerator() method:
	IEnumerator* IEnumerable::GetEnumerator()
	{
		return GetEnumerator();
	}

public:
	LuaState* m_state;
	LuaPlus::LuaObject __nogc* m_object;
};



public __gc class LuaStackObject
{
public:
	LuaStackObject(ManagedLuaPlus::LuaState* state, int index);
	LuaStackObject(ManagedLuaPlus::LuaState* state, LuaPlus::LuaStackObject obj);

	~LuaStackObject()
	{
		delete m_object;
	}

	__property String* get_WatchType()
	{
		int type = GetType();

		switch (type)
		{
			case LUA_TNONE:
				return new String("NONE");

			case LUA_TNIL:
				return new String("NIL");

			case LUA_TBOOLEAN:
			{
				String* str = String::Format("BOOLEAN: {0}", GetBoolean() ? S"true" : S"false");
				return str;
			}

			case LUA_TLIGHTUSERDATA:
			{
				String* str = String::Format("LIGHTUSERDATA: {0:X}", __box((size_t)GetLightUserData()));
				return str;
			}

			case LUA_TNUMBER:
			{
				String* str = String::Format("NUMBER: {0:G}", __box(GetNumber()));
				return str;
			}

			case LUA_TSTRING:
			{
				String* str = String::Format("STRING: {0}", GetString());
				return str;
			}

			case LUA_TTABLE:
			{
				String* str = new String("TABLE");
				return str;
			}

			case LUA_TFUNCTION:
			{
				String* str = new String("FUNCTION");
				return str;
			}

			case LUA_TUSERDATA:
			{
				String* str = String::Format("USERDATA: {0:X}", __box((size_t)GetUserData()));
				return str;
			}

			case LUA_TTHREAD:
			{
				String* str = new String("FORMAT");
				return str;
			}

			case LUA_TWSTRING:
			{
				String* str = String::Format(L"WSTRING: {0}", GetWString());
				return str;
			}
		}

		return new String("Unknown");
	}

	static int op_Implicit(ManagedLuaPlus::LuaStackObject* obj)
	{
		return (int)obj->m_object->m_stackIndex;
	}

	/**
		Various constructors accepting different parameters.
	**/
/*	LuaStackObject(lua_State* state, int stackIndex) :
		m_state(state),
		m_stackIndex(stackIndex)
	{
	}
*/
	/**
		Various constructors accepting different parameters.
	**/
//	LuaStackObject(LuaState* state, int stackIndex);

	/**
		Various constructors accepting different parameters.
	**/
//	LuaStackObject(LuaState& state, int stackIndex);

	/**
		Copy constructor.
	**/
/*	LuaStackObject(const LuaStackObject& src) :
		m_state(src.m_state),
		m_stackIndex(src.m_stackIndex)
	{
	}
*/
	/**
		Assignment operator.
	**/
/*	const LuaStackObject& operator=(const LuaStackObject& src)
	{
		m_state = src.m_state;
		m_stackIndex = src.m_stackIndex;
		return *this;
	}
*/
	/**
		Retrieves the LuaState object associated with this LuaStackObject.
	**/
	LuaState* GetLuaState()				{  return m_state;  }

	String* GetTypeName()				{  return new String(m_object->GetTypeName());  }
	int GetType()						{  return m_object->GetType();  }

	bool IsNil()						{  return m_object->IsNil();  }
	bool IsTable()						{  return m_object->IsTable();  }
	bool IsUserData()					{  return m_object->IsUserData();  }
	bool IsCFunction()					{  return m_object->IsCFunction();  }
	bool IsNumber()						{  return m_object->IsNumber();  }
	bool IsString()						{  return m_object->IsString();  }
	bool IsWString()					{  return m_object->IsWString();  }
	bool IsFunction()					{  return m_object->IsFunction();  }
	bool IsNone()						{  return m_object->IsNone();  }
	bool IsLightUserData()				{  return m_object->IsLightUserData();  }
	bool IsBoolean()					{  return m_object->IsBoolean();  }

	int GetInteger()					{  return m_object->GetInteger();  }
	float GetFloat()					{  return m_object->GetFloat();  }
	double GetDouble()					{  return m_object->GetNumber();  }
	lua_Number GetNumber()				{  return m_object->GetNumber();  }
	String* GetString()					{  return new String(m_object->GetString());  }
	String* GetWString()				{  return new String((const wchar_t*)m_object->GetWString());  }
	int StrLen()						{  return m_object->StrLen();  }
	lua_CFunction GetCFunction()		{  return m_object->GetCFunction();  }
	void* GetUserData()					{  return m_object->GetUserData();  }
	const void* GetLuaPointer()			{  return m_object->GetLuaPointer();  }
	void* GetLightUserData()			{  return m_object->GetLightUserData();  }
	bool GetBoolean()					{  return m_object->GetBoolean();  }

	void Push()							{  return m_object->Push();  }
	void Pop()							{  return m_object->Pop();  }

	LuaStackObject* GetMetaTable()			{  return new LuaStackObject(m_state, m_object->GetMetaTable());  }
	void SetMetaTable()					{  m_object->SetMetaTable();  }
	void SetMetaTable(LuaStackObject* value){  m_object->SetMetaTable(*value->m_object);  }

	void SetTable()						{  m_object->SetTable();  }
	int GetCount()						{  return m_object->GetCount();  }

	LuaStackObject* CreateTable(String* name)
		{  return new LuaStackObject(m_state, m_object->CreateTable(MarshalStringANSI(name)));  }
	LuaStackObject* CreateTable(int index)
		{  return new LuaStackObject(m_state, m_object->CreateTable(index));  }

	void SetNil(String* name)									{  m_object->SetNil(MarshalStringANSI(name));  }
	void SetNil(int index)										{  m_object->SetNil(index);  }
	void SetBoolean(String* name, bool value)					{  m_object->SetBoolean(MarshalStringANSI(name), value);  }
	void SetBoolean(int index, bool value)						{  m_object->SetBoolean(index, value);  }
	void SetNumber(String* name, double value)					{  m_object->SetNumber(MarshalStringANSI(name), (lua_Number)value);  }
	void SetNumber(int index, double value)						{  m_object->SetNumber(index, (lua_Number)value);  }
	void SetString(String* name, String* value)					{  m_object->SetString(MarshalStringANSI(name), MarshalStringANSI(value));  }
	void SetString(int index, String* value)					{  m_object->SetString(index, MarshalStringANSI(value));  }
	void SetWString(String* name, String* value)				{  m_object->SetWString(MarshalStringANSI(name), MarshalStringUnicode(value));  }
	void SetWString(int index, String* value)					{  m_object->SetWString(index, MarshalStringUnicode(value));  }
	void SetUserData(String* name, void* value)					{  m_object->SetUserData(MarshalStringANSI(name), value);  }
	void SetUserData(int index, void* value)					{  m_object->SetUserData(index, value);  }
	void SetLightUserData(String* name, void* value)			{  m_object->SetLightUserData(MarshalStringANSI(name), value);  }
	void SetObject(String* name, LuaStackObject* value)			{  m_object->SetObject(MarshalStringANSI(name), *value->m_object);  }
	void SetObject(int index, LuaStackObject* value)			{  m_object->SetObject(index, *value->m_object);  }

	LuaStackObject* GetByName(String* name)						{  return new LuaStackObject(m_state, m_object->GetByName(MarshalStringANSI(name)));  }
	LuaStackObject* GetByIndex(int index)							{  return new LuaStackObject(m_state, m_object->GetByIndex(index));  }
	LuaStackObject* GetByObject(LuaStackObject* obj)					{  return new LuaStackObject(m_state, m_object->GetByObject(*obj->m_object));  }

public:
	LuaState* m_state;
	LuaPlus::LuaStackObject __nogc* m_object;
};


public __gc interface LuaStateOutFile
{
public:
	virtual void Print(Byte /*message*/[])
	{
	}
};


public __gc class LuaStateStreamOutFile : public LuaStateOutFile
{
public:
	LuaStateStreamOutFile(System::IO::Stream* stream) :
		m_stream(stream)
	{
	}

	virtual void Print(Byte str[])
	{
		m_stream->Write(str, 0, str->Length);
	}

private:
	System::IO::Stream* m_stream;
};


static void PrintShim(gcroot<ManagedLuaPlus::LuaStateOutFile*>& file, const char* buffer)
{
	Byte managedBuffer[] = new Byte[(int)strlen(buffer)];
	Marshal::Copy(IntPtr((void*)buffer), managedBuffer, 0, managedBuffer->Length);
	file->Print(managedBuffer);
}


class MarshalLuaStateOutFile : public LuaPlus::LuaStateOutFile
{
public:
	MarshalLuaStateOutFile(ManagedLuaPlus::LuaStateOutFile* file) :
		m_file(file)
	{
	}

	virtual void Print(const char* str, ...)
	{
		char message[800];
		va_list arglist;

		va_start(arglist, str);
		vsprintf(message, str, arglist);
		va_end(arglist);

		PrintShim(m_file, message);
	}

private:
	gcroot<ManagedLuaPlus::LuaStateOutFile*> m_file;
};



public __gc class LuaState
{
public:
	__value enum ErrorCodes
	{
		/* error codes for `lua_load' and `lua_pcall' */
		ERRRUN = 1,
		ERRFILE = 2,
		ERRSYNTAX = 3,
		ERRMEM = 4,
		ERRERR = 5,
	};

	__value enum Types
	{
		TNONE = (-1),
		TNIL	= 0,
		TBOOLEAN	= 1,
		TLIGHTUSERDATA	= 2,
		TNUMBER = 3,
		TSTRING = 4,
		TTABLE = 5,
		TFUNCTION = 6,
		TUSERDATA = 7,
		TTHREAD = 8,
		TWSTRING = 9,
		NTYPES = 11,
	};

	__value enum DumpObjectTypes
	{
		DUMP_ALPHABETICAL		= 0x00000001,
		DUMP_WRITEALL			= 0x00000002,
		DUMP_WRITETABLEPOINTERS = 0x00000004,
	};

	LuaState();
	LuaState(bool initStandardLibrary);
	LuaState(IntPtr state);
	~LuaState();

	void OpenLibs()								{  m_state->OpenLibs();  }

	// LuaObject
	LuaObject* NewObject()						{  return new LuaObject(this);  }

	// Stack functions.
	LuaStackObject* Stack(int index)			{  return new LuaStackObject(this, index);  }
	LuaStackObject* StackTop()	        		{  return new LuaStackObject(this, m_state->GetTop());  }
	int GetTop()								{  return m_state->GetTop();  }
	void SetTop(int index)						{  m_state->SetTop(index);  }
	void PushValue(int index)					{  m_state->PushValue(index);  }
	void PushValue(LuaStackObject* object)		{  m_state->PushValue(object->m_object->m_stackIndex);  }
	void Remove(int index)						{  m_state->Remove(index);  }
	void Insert(int index)						{  m_state->Insert(index);  }
	void Replace(int index)						{  m_state->Replace(index);  }
	int CheckStack(int size)					{  return m_state->CheckStack(size);  }
	void XMove(LuaState* to, int n)				{  m_state->XMove(to->m_state, n);  }
	int Equal(int index1, int index2)			{  return m_state->Equal(index1, index2);  }
	int Equal(const LuaObject* o1, const LuaObject* o2)
		{  return m_state->Equal(*o1->m_object, *o2->m_object);  }
	int RawEqual(int index1, int index2)		{  return m_state->RawEqual(index1, index2);  }
	int LessThan(int index1, int index2)		{  return m_state->LessThan(index1, index2);  }
	int LessThan(const LuaObject* o1, const LuaObject* o2)
		{  return m_state->LessThan(*o1->m_object, *o2->m_object);  }

	// push functions (C -> stack)
	LuaStackObject* PushNil()					{  return new LuaStackObject(this, m_state->PushNil());  }
	LuaStackObject* PushInteger(int n)		    {  return new LuaStackObject(this, m_state->PushInteger(n));  }
	LuaStackObject* PushNumber(double n)		{  return new LuaStackObject(this, m_state->PushNumber((lua_Number)n));  }
	LuaStackObject* PushLString(String* s, size_t len)	{  return new LuaStackObject(this, m_state->PushLString(MarshalStringANSI(s), len));  }
	LuaStackObject* PushLWString(String* s, size_t len)	{  return new LuaStackObject(this, m_state->PushLWString(MarshalStringUnicode(s), len));  }
	LuaStackObject* PushString(String* s)		{  return new LuaStackObject(this, m_state->PushString(MarshalStringANSI(s)));  }
	LuaStackObject* PushWString(String* s)		{  return new LuaStackObject(this, m_state->PushWString(MarshalStringUnicode(s)));  }
//LUA_API const char *m_state->pushvfstring (State *L, const char *fmt,
//                                                    va_list argp);
//LUA_API const char *m_state->pushfstring (State *L, const char *fmt, ...);
//	void PushCClosure(CFunction fn, int n){  m_state->pushcclosure(fn, n);  }
//	void PushCClosure(LuaStateCFunction fn, int n)	{  m_state->pushcclosure((CFunction)fn, n);  }
//	void PushCFunction(CFunction f)		{  m_state->pushcclosure(f, 0);  }
//	void PushCFunction(LuaStateCFunction f)	{  m_state->pushcclosure((CFunction)f, 0);  }
	LuaStackObject* PushBoolean(bool value)		{  return new LuaStackObject(this, m_state->PushBoolean(value));  }
	LuaStackObject* PushLightUserData(void* p)	{  return new LuaStackObject(this, m_state->PushLightUserData(p));  }
	LuaStackObject* PushThread()				{  return new LuaStackObject(this, m_state->PushThread());  }

	LuaStackObject* CreateTable()				{  return new LuaStackObject(this, m_state->CreateTable());  }
	LuaStackObject* CreateTable(int narr)		{  return new LuaStackObject(this, m_state->CreateTable(narr));  }
	LuaStackObject* CreateTable(int narr, int nrec)	{  return new LuaStackObject(this, m_state->CreateTable(narr, nrec));  }
	LuaStackObject* NewUserData(size_t size)	{  return new LuaStackObject(this, m_state->NewUserData(size));  }

	// get functions (Lua -> stack)
	void GetTable(int index)					{  m_state->GetTable(index);  }
	void RawGet(int index)						{  m_state->RawGet(index);  }
	void RawGetI(int index, int n)				{  m_state->RawGetI(index, n);  }
	LuaStackObject* GetMetaTable(int index)		{  return new LuaStackObject(this, m_state->GetMetaTable(index));  }

	// LuaPlus ---->
	__property LuaObject* get_Globals()			{  return GetGlobals();  }
	LuaObject* GetGlobals()						{  return new LuaObject(this, m_state->GetGlobals());  }
	LuaStackObject* GetGlobals_Stack()			{  return new LuaStackObject(this, LUA_GLOBALSINDEX);  }
	__property LuaObject* get_Registry()		{  return GetRegistry();  }
	LuaObject* GetRegistry()					{  return new LuaObject(this, m_state->GetRegistry());  }
	LuaStackObject* GetRegistry_Stack()			{  return new LuaStackObject(this, LUA_REGISTRYINDEX);  }
	LuaObject* GetGlobal(String* name)			{  return new LuaObject(this, m_state->GetGlobal(MarshalStringANSI(name)));  }
	LuaStackObject* GetGlobal_Stack(String* name){  return new LuaStackObject(this, m_state->GetGlobal_Stack(MarshalStringANSI(name)));  }
	// set functions(stack -> Lua)
	void SetTable(int index)					{  m_state->SetTable(index);  }
	void RawSet(int index)						{  m_state->RawSet(index);  }
	void RawSetI(int index, int n)				{  m_state->RawSetI(index, n);  }
	void SetMetaTable(int index)				{  m_state->SetMetaTable(index);  }

	// `load' and `call' functions (load and run Lua code)
	void Call(int nargs, int nresults)			{  m_state->Call(nargs, nresults);  }
	int PCall(int nargs, int nresults, int errf){  return m_state->PCall(nargs, nresults, errf);  }
//	int CPCall(lua_CFunction func, void* ud)	{  return m_state->CPCall(func, ud);  }
//	int Load(luaplus_Reader reader, void *dt, const char *chunkname);
//	int WLoad(luaplus_Reader reader, void *data, const char *chunkname);

//	int Dump(lua_Chunkwriter writer, void* data){  return m_state->Dump(writer, data);  }

	// Coroutine functions
	int CoYield(int nresults)					{  return m_state->CoYield(nresults);  }
	int CoResume(int narg)						{  return m_state->CoResume(narg);  }
	int CoStatus()								{  return m_state->CoStatus();  }

	// garbage-collection function and options
	int GC(int what, int data)					{  return m_state->GC(what, data);  }

	// Miscellaneous functions
	int Error()									{  return m_state->Error();  }

	int Next(int index)							{  return m_state->Next(index);  }

	void Concat(int n)							{  m_state->Concat(n);  }

//	lua_Alloc GetAllocF(void **ud);
//	void SetAllocF(lua_Alloc f, void *ud);

	// Helper functions
	void Pop()									{  m_state->Pop(1);  }
	void Pop(int amount)						{  m_state->Pop(amount);  }

	// debug functions
//	int GetStack(int level, lua_Debug* ar);
//	int GetInfo(const char* what, lua_Debug* ar);
//	const char* GetLocal(const lua_Debug* ar, int n);
//	const char* SetLocal(const lua_Debug* ar, int n);
//	const char* GetUpValue(int funcindex, int n);
//	const char* SetUpValue(int funcindex, int n);

//	int SetHook(lua_Hook func, int mask, int count);
//	lua_Hook GetHook();
//	int GetHookMask();
//	int GetHookCount();

	// lauxlib functions.
	int TypeError(int narg, String* tname)								{  return m_state->TypeError(narg, MarshalStringANSI(tname));  }
	int ArgError(int numarg, String* extramsg)							{  return m_state->ArgError(numarg, MarshalStringANSI(extramsg));  }
	const char* CheckLString(int numArg, size_t* len)					{  return m_state->CheckLString(numArg, len);  }
	const char* OptLString(int numArg, const char *def, size_t* len)	{  return m_state->OptLString(numArg, def, len);  }
	lua_Number CheckNumber(int numArg)									{  return m_state->CheckNumber(numArg);  }
	lua_Number OptNumber(int nArg, lua_Number def)						{  return m_state->OptNumber(nArg, def);  }

	int CheckInteger(int numArg)										{  return (int)m_state->CheckInteger(numArg);  }
	int OptInteger(int numArg, int def)									{  return (int)m_state->OptInteger(numArg, def);  }

	void CheckStack(int sz, const char* msg)							{  m_state->CheckStack(sz, msg);  }
	void CheckType(int narg, int t)										{  m_state->CheckType(narg, t);  }
	void CheckAny(int narg)												{  m_state->CheckAny(narg);  }

	LuaStackObject* NewMetaTable(String* tname)							{  return new LuaStackObject(this, m_state->NewMetaTable(MarshalStringANSI(tname)));  }
	void* CheckUData(int ud, String* tname)								{  return m_state->CheckUData(ud, MarshalStringANSI(tname));  }

	int Where(int lvl)													{  return m_state->Where(lvl);  }
//	void Error(const char* fmt, ...);

//	int CheckOption(int narg, const char *def, const char *const lst[]);

	int Ref(int t)														{  return m_state->Ref(t);  }
	void Unref(int t, int ref)											{  m_state->Unref(t, ref);  }

	int LoadFile(String* filename)										{  return m_state->LoadFile(MarshalStringANSI(filename));  }
	int LoadBuffer(String* buff, size_t size, String* name)				{  return m_state->LoadBuffer(MarshalStringANSI(buff), size, MarshalStringANSI(name));  }
	int LoadString(String* str)		        							{  return m_state->LoadString(MarshalStringANSI(str));  }

	String* GSub(String *s, String *p, String *r)						{  return new String(m_state->GSub(MarshalStringANSI(s), MarshalStringANSI(p), MarshalStringANSI(r)));  }

	String* FindTable(int idx, String *fname, int szhint)				{  return new String(m_state->FindTable(idx, MarshalStringANSI(fname), szhint));  }

	void ArgCheck(bool condition, int numarg, String* extramsg)			{  m_state->ArgCheck(condition, numarg, MarshalStringANSI(extramsg));  }
	String* CheckString(int numArg)										{  return new String(m_state->CheckString(numArg));  }
	String* OptString(int numArg, const char* def)						{  return new String(m_state->OptString(numArg, def));  }
	int CheckInt(int numArg)											{  return m_state->CheckInt(numArg);  }
	int OptInt(int numArg, int def)										{  return m_state->OptInt(numArg, def);  }
	long CheckLong(int numArg)											{  return m_state->CheckLong(numArg);  }
	long OptLong(int numArg, int def)									{  return m_state->OptLong(numArg, def);  }
//	void* CheckUData(int ud, const char* tname)							{  return m_state->CheckUData(ud, tname);  }

	int DoFile(String* filename)				{  return m_state->DoFile(MarshalStringANSI(filename));  }
    int DoFile(String* filename, LuaObject* fenvObj)
    {
        return m_state->DoFile(MarshalStringANSI(filename), *fenvObj->m_object);
    }
	int DoString(String* str)					{  return m_state->DoString(MarshalStringANSI(str));  }
	int DoString(String* str, LuaObject* fenvObj)
    {
        return m_state->DoString(MarshalStringANSI(str), *fenvObj->m_object);
    }
	int DoBuffer(String* buff, size_t size, String* name)
	{
		return m_state->DoBuffer(MarshalStringANSI(buff), size, MarshalStringANSI(name));

	}
	int DoBuffer(String* buff, size_t size, String* name, LuaObject* fenvObj)
	{
		return m_state->DoBuffer(MarshalStringANSI(buff), size, MarshalStringANSI(name), *fenvObj->m_object);
	}

//	int LoadWBuffer(const lua_WChar* buff, size_t size, const char* name);
//	int LoadWString(const lua_WChar* str);

//	int DoWString(const lua_WChar* str, const char* name);
//	int DoWBuffer(const lua_WChar* buff, size_t size, const char *name);

	LuaObject* NewUserDataBox(void* u)			{  return new LuaObject(this, m_state->NewUserDataBox(u));  }

	bool DumpObject(String* filename, String* name, LuaObject* value)
	{
		return m_state->DumpObject(MarshalStringANSI(filename), MarshalStringANSI(name),
			*value->m_object, DUMP_ALPHABETICAL, 0, 0xffffffff);
	}

	bool DumpObject(String* filename, String* name, LuaObject* value,
						 unsigned int flags, int indentLevel, unsigned int maxIndentLevel)
	{
		return m_state->DumpObject(MarshalStringANSI(filename), MarshalStringANSI(name),
			*value->m_object, flags, indentLevel, maxIndentLevel);
	}

	bool DumpObject(String* filename, LuaObject* key, LuaObject* value,
						 unsigned int flags, int indentLevel, unsigned int maxIndentLevel)
	{
		return m_state->DumpObject(MarshalStringANSI(filename), *key->m_object,
			*value->m_object, flags, indentLevel, maxIndentLevel);
	}

	bool DumpObject(LuaStateOutFile* file, String* name, LuaObject* value)
	{
		MarshalLuaStateOutFile marshalFile(file);
		return m_state->DumpObject(marshalFile, MarshalStringANSI(name),
			*value->m_object, DUMP_ALPHABETICAL, 0, 0xffffffff);
	}

	bool DumpObject(LuaStateOutFile* file, String* name, LuaObject* value,
						 unsigned int flags, int indentLevel, unsigned int maxIndentLevel)
	{
		MarshalLuaStateOutFile marshalFile(file);
		return m_state->DumpObject(marshalFile, MarshalStringANSI(name),
			*value->m_object, flags, indentLevel, maxIndentLevel);
	}

	bool DumpObject(LuaStateOutFile* file, LuaObject* key, LuaObject* value,
						 unsigned int flags, int indentLevel, unsigned int maxIndentLevel)
	{
		MarshalLuaStateOutFile marshalFile(file);
		return m_state->DumpObject(marshalFile, *key->m_object,
			*value->m_object, flags, indentLevel, maxIndentLevel);
	}

	bool DumpGlobals(String* filename, unsigned int flags, unsigned int maxIndentLevel)
	{
		return m_state->DumpGlobals(MarshalStringANSI(filename), flags, maxIndentLevel);
	}

	bool DumpGlobals(String* filename)
	{
		return m_state->DumpGlobals(MarshalStringANSI(filename), DUMP_ALPHABETICAL, 0xffffffff);
	}

	// Extra
	LuaStackObject* BoxPointer(void* u)									{  return new LuaStackObject(this, m_state->BoxPointer(u));  }
	void* UnBoxPointer(int stackIndex)									{  return m_state->UnBoxPointer(stackIndex);  }

	int UpValueIndex(int i)												{  return m_state->UpValueIndex(i);  }

	LuaObject* GetLocalByName( int level, String* name )				{  return new LuaObject(this, m_state->GetLocalByName(level, MarshalStringANSI(name)));  }

	// LuaPlus ------->
    void SetFEnv( int index )											{  m_state->SetFEnv(index);  }
//	void SetGlobals(int index)					{  m_state->SetGlobals(index);  }
//	void SetGlobal(String* name)				{  m_state->SetGlobal(MarshalStringANSI(name));  }




//	LuaStackObject* GetRef(int ref)				{  return new LuaStackObject(this, m_state->GetRef(ref));  }

	LuaPlus::LuaState __nogc* m_state;
	Hashtable* m_objects;
	bool m_ownState;

	// support for loader-lock issues
	// see http://support.microsoft.com/?id=814472
/*	static BOOL Initialize()
	{
		BOOL retval = TRUE;
		try
		{
			retval =  __crt_dll_initialize();
		}
		catch(System::Exception* e)
		{
			Console::WriteLine(e->Message);
			retval = FALSE;
		}
		return retval;
	}
*/
	// support for loader-lock issues
	// see http://support.microsoft.com/?id=814472
/*	static BOOL Terminate()
	{
		BOOL retval = TRUE;
		try
		{
			retval = __crt_dll_terminate();
		}
		catch(System::Exception* e)
		{
			Console::WriteLine(e->Message);
			retval = FALSE;
		}
		return retval;
	}
*/
};


public __gc class LuaTableIterator
{
public:
	LuaTableIterator(LuaObject* tableObj) :
		m_tableObj(tableObj)
	{
		m_it = new LuaPlus::LuaTableIterator(*tableObj->m_object, true);
		if (!tableObj->IsTable())
			throw new LuaException(new String("Object must be a table to enumerate over"));
	}

	LuaTableIterator(LuaObject* tableObj, bool doReset) :
		m_tableObj(tableObj)
	{
		m_it = new LuaPlus::LuaTableIterator(*tableObj->m_object, doReset);
		if (!tableObj->IsTable())
			throw new LuaException(new String("Object must be a table to enumerate over"));
	}

	~LuaTableIterator()
	{
		delete m_it;
	}

	void Reset()
	{
		m_it->Reset();
	}

	void Invalidate()
	{
		m_it->Invalidate();
	}

	bool Next()
	{
		return m_it->Next();
	}

	bool IsValid()
	{
		return m_it->IsValid();
	}

	static LuaTableIterator* op_Increment(LuaTableIterator* it)
	{
		it->Next();
		return it;
	}

	LuaObject* GetKey()
	{
		return new LuaObject(TableObj->GetState(), m_it->GetKey());
	}


	LuaObject* GetValue()
	{
		return new LuaObject(TableObj->GetState(), m_it->GetValue());
	}

protected:
	__property LuaObject* get_TableObj()
	{
		return static_cast<LuaObject*>(m_tableObj);
	}

private:
	LuaPlus::LuaTableIterator __nogc* m_it;
	LuaObject* m_tableObj;
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
LuaStackObject::LuaStackObject(ManagedLuaPlus::LuaState* state, int index) :
	m_state(state),
	m_object(new LuaPlus::LuaStackObject(state->m_state, index))
{
}


LuaStackObject::LuaStackObject(ManagedLuaPlus::LuaState* state, LuaPlus::LuaStackObject obj) :
	m_state(state),
	m_object(new LuaPlus::LuaStackObject(obj))
{
}


LuaState::LuaState() :
	m_objects(new Hashtable())
{
	m_state = LuaPlus::LuaState::Create();
	m_ownState = true;
}


LuaState::LuaState(bool initStandardLibrary) :
	m_objects(new Hashtable())
{
	m_state = LuaPlus::LuaState::Create(initStandardLibrary);
	m_ownState = true;
}


LuaState::LuaState(IntPtr state) :
	m_objects(new Hashtable())
{
	m_state = (LuaPlus::LuaState*)state.ToPointer();
	m_ownState = false;
}


LuaState::~LuaState()
{
	try
	{
		if (m_objects  &&  m_objects->Count > 0)
		{
			IDictionaryEnumerator __gc *myEnumerator = m_objects->GetEnumerator();
			while (myEnumerator->MoveNext())
			{
				LuaObject* obj = dynamic_cast<LuaObject*>(myEnumerator->Key);
				if (obj)
				{
					delete obj->m_object;
					obj->m_object = NULL;
					obj->m_state = NULL;
				}
			}

			m_objects->Clear();
			m_objects = NULL;
		}
		System::GC::Collect();
		System::GC::WaitForPendingFinalizers();
		if (m_state  &&  m_ownState)
			LuaPlus::LuaState::Destroy(m_state);
	}
	catch (const LuaPlus::LuaException& e)
	{
		throw new LuaException(new String(e.GetErrorMessage()));
	}
}


/**
	A helper class for automatically setting the stack state back to point of
	LuaAutoBlock creation.
**/
__gc class LuaAutoBlock
{
public:
	LuaAutoBlock(LuaState* state) :
		m_state(state)
	{
		m_stackTop = m_state->GetTop();
	}

	LuaAutoBlock(LuaStackObject* object) :
		m_state(object->m_state)
	{
		m_stackTop = m_state->GetTop();
	}

	~LuaAutoBlock()
	{
		m_state->SetTop(m_stackTop);
	}

private:
	LuaState* m_state;
	int m_stackTop;
};


LuaObject::LuaObject() :
	m_object(new LuaPlus::LuaObject)
{
}

	
LuaObject::LuaObject(ManagedLuaPlus::LuaState* state) :
	m_state(state),
	m_object(new LuaPlus::LuaObject(state->m_state))
{
	GetState()->m_objects->Add(this, __box(true));
}

LuaObject::LuaObject(ManagedLuaPlus::LuaState* state, int stackIndex) :
	m_state(state),
	m_object(new LuaPlus::LuaObject(state->m_state, stackIndex))
{
	GetState()->m_objects->Add(this, __box(true));
}

LuaObject::LuaObject(ManagedLuaPlus::LuaObject* object) :
	m_state(object->GetState()),
	m_object(new LuaPlus::LuaObject(*object->m_object))
{
	GetState()->m_objects->Add(this, __box(true));
}

LuaObject::LuaObject(ManagedLuaPlus::LuaState* state, LuaPlus::LuaObject& object) :
	m_state(state),
	m_object(new LuaPlus::LuaObject(object))
{
	GetState()->m_objects->Add(this, __box(true));
}


LuaObject::LuaObject(ManagedLuaPlus::LuaStackObject* object) :
	m_state(object->m_state),
	m_object(new LuaPlus::LuaObject(object->m_object->m_state, *object->m_object))
{
	GetState()->m_objects->Add(this, __box(true));
}


LuaStackObject* LuaObject::Push()
{
	return new LuaStackObject(GetState(), m_object->Push());
}


LuaObject* LuaObject::GetByObject(LuaStackObject* obj)
{
	try
	{
		return new LuaObject(GetState(), m_object->GetByObject(*obj->m_object));
	}
	catch (const LuaPlus::LuaException& e)
	{
		throw new LuaException(new String(e.GetErrorMessage()));
	}
}


LuaObject* LuaObject::RawGetByObject(LuaStackObject* obj)
{
	try
	{
		return new LuaObject(GetState(), m_object->RawGetByObject(*obj->m_object));
	}
	catch (const LuaPlus::LuaException& e)
	{
		throw new LuaException(new String(e.GetErrorMessage()));
	}
}


void LuaObject::AssignNil(ManagedLuaPlus::LuaState* state)
{
	try
	{
		if (GetState())
			GetState()->m_objects->Remove(this);
		m_object->AssignNil(state->m_state);
		m_state = state;
		if (state)
			GetState()->m_objects->Add(this, __box(true));
	}
	catch (const LuaPlus::LuaException& e)
	{
		throw new LuaException(new String(e.GetErrorMessage()));
	}
}

void LuaObject::AssignBoolean(ManagedLuaPlus::LuaState* state, bool value)
{
	try
	{
		if (GetState())
			GetState()->m_objects->Remove(this);
		m_object->AssignBoolean(state->m_state, value);
		m_state = state;
		if (state)
			GetState()->m_objects->Add(this, __box(true));
	}
	catch (const LuaPlus::LuaException& e)
	{
		throw new LuaException(new String(e.GetErrorMessage()));
	}
}

void LuaObject::AssignNumber(ManagedLuaPlus::LuaState* state, lua_Number value)
{
	try
	{
		if (GetState())
			GetState()->m_objects->Remove(this);
		m_object->AssignNumber(state->m_state, value);
		m_state = state;
		if (state)
			GetState()->m_objects->Add(this, __box(true));
	}
	catch (const LuaPlus::LuaException& e)
	{
		throw new LuaException(new String(e.GetErrorMessage()));
	}
}

void LuaObject::AssignString(ManagedLuaPlus::LuaState* state, String* value)
{
	try
	{
		if (GetState())
			GetState()->m_objects->Remove(this);
		m_object->AssignString(state->m_state, MarshalStringANSI(value));
		m_state = state;
		if (state)
			GetState()->m_objects->Add(this, __box(true));
	}
	catch (const LuaPlus::LuaException& e)
	{
		throw new LuaException(new String(e.GetErrorMessage()));
	}
}

void LuaObject::AssignString(ManagedLuaPlus::LuaState* state, String* value, unsigned int len)
{
	try
	{
		if (GetState())
			GetState()->m_objects->Remove(this);
		m_object->AssignString(state->m_state, MarshalStringANSI(value), len);
		m_state = state;
		if (state)
			GetState()->m_objects->Add(this, __box(true));
	}
	catch (const LuaPlus::LuaException& e)
	{
		throw new LuaException(new String(e.GetErrorMessage()));
	}
}

void LuaObject::AssignWString(ManagedLuaPlus::LuaState* state, String* value)
{
	try
	{
		if (GetState())
			GetState()->m_objects->Remove(this);
		m_object->AssignWString(state->m_state, MarshalStringUnicode(value));
		m_state = state;
		if (state)
			GetState()->m_objects->Add(this, __box(true));
	}
	catch (const LuaPlus::LuaException& e)
	{
		throw new LuaException(new String(e.GetErrorMessage()));
	}
}

void LuaObject::AssignWString(ManagedLuaPlus::LuaState* state, String* value, unsigned int len)
{
	try
	{
		if (GetState())
			GetState()->m_objects->Remove(this);
		m_object->AssignWString(state->m_state, MarshalStringUnicode(value), len);
		m_state = state;
		if (state)
			GetState()->m_objects->Add(this, __box(true));
	}
	catch (const LuaPlus::LuaException& e)
	{
		throw new LuaException(new String(e.GetErrorMessage()));
	}
}

void LuaObject::AssignUserData(ManagedLuaPlus::LuaState* state, void* value)
{
	try
	{
		if (GetState())
			GetState()->m_objects->Remove(this);
		m_object->AssignUserData(state->m_state, value);
		m_state = state;
		if (state)
			GetState()->m_objects->Add(this, __box(true));
	}
	catch (const LuaPlus::LuaException& e)
	{
		throw new LuaException(new String(e.GetErrorMessage()));
	}
}

void LuaObject::AssignLightUserData(ManagedLuaPlus::LuaState* state, void* value)
{
	try
	{
		if (GetState())
			GetState()->m_objects->Remove(this);
		m_object->AssignLightUserData(state->m_state, value);
		m_state = state;
		if (state)
			GetState()->m_objects->Add(this, __box(true));
	}
	catch (const LuaPlus::LuaException& e)
	{
		throw new LuaException(new String(e.GetErrorMessage()));
	}
}

void LuaObject::AssignObject(LuaObject* value)
{
	try
	{
		if (GetState())
			GetState()->m_objects->Remove(this);
		m_object->AssignObject(*value->m_object);
		m_state = value->m_state;
		if (GetState())
			GetState()->m_objects->Add(this, __box(true));
	}
	catch (const LuaPlus::LuaException& e)
	{
		throw new LuaException(new String(e.GetErrorMessage()));
	}
}

void LuaObject::AssignNewTable(ManagedLuaPlus::LuaState* state)
{
	try
	{
		if (GetState())
			GetState()->m_objects->Remove(this);
		m_object->AssignNewTable(state->m_state);
		m_state = state;
		if (state)
			GetState()->m_objects->Add(this, __box(true));
	}
	catch (const LuaPlus::LuaException& e)
	{
		throw new LuaException(new String(e.GetErrorMessage()));
	}
}


LuaObject* LuaObject::get_Item(LuaStackObject* obj)
{
	try
	{
		return new LuaObject(GetState(), m_object->RawGetByObject(*obj->m_object));
	}
	catch (const LuaPlus::LuaException& e)
	{
		throw new LuaException(new String(e.GetErrorMessage()));
	}
}

static int CallbackDispatcher(lua_State* inState)
{
	IntPtr callbackHandle = (IntPtr)lua_touserdata(inState, lua_upvalueindex(1));
	IntPtr stateHandle = (IntPtr)lua_touserdata(inState, lua_upvalueindex(2));
	ManagedLuaPlus::LuaPlusCallback* callback = __try_cast<ManagedLuaPlus::LuaPlusCallback*>(GCHandle::op_Explicit(callbackHandle).Target);
	ManagedLuaPlus::LuaState* state = __try_cast<ManagedLuaPlus::LuaState*>(GCHandle::op_Explicit(stateHandle).Target);
	return callback(state);
}


static int OldCallbackDispatcher(lua_State* inState)
{
	IntPtr callbackHandle = (IntPtr)lua_touserdata(inState, lua_upvalueindex(1));
	IntPtr stateHandle = (IntPtr)lua_touserdata(inState, lua_upvalueindex(2));
	ManagedLuaPlus::LuaPlusOldCallback* callback = __try_cast<ManagedLuaPlus::LuaPlusOldCallback*>(GCHandle::op_Explicit(callbackHandle).Target);
	ManagedLuaPlus::LuaState* state = __try_cast<ManagedLuaPlus::LuaState*>(GCHandle::op_Explicit(stateHandle).Target);
	const int MAX_ARGS = 20;
	ManagedLuaPlus::LuaStackObject* args[] = new ManagedLuaPlus::LuaStackObject*[MAX_ARGS + 1];
	for (int i = 1; i <= MAX_ARGS; ++i)
	{
		args[i] = new ManagedLuaPlus::LuaStackObject(state, i);
	}
	return callback(state, args);
}


static int GCHandle_gc(lua_State* state)
{
	if (lua_isuserdata(state, 1))
	{
		GCHandle::op_Explicit((IntPtr)lua_touserdata(state, 1)).Free();
	}

	return 0;
}


LuaPlus::LuaObject CreateGCHandleObj(LuaPlus::LuaState* state, Object* theObj)
{
	LuaPlus::LuaObject metaTableObj = state->GetRegistry()["GCHandleMetaTable"];
	if (metaTableObj.IsNil())
	{
		metaTableObj = state->GetRegistry().CreateTable("GCHandleMetaTable");
		metaTableObj.Register("__gc", GCHandle_gc);
	}

	LuaPlus::LuaObject outObj = state->NewUserDataBox(GCHandle::op_Explicit(GCHandle::Alloc(theObj)).ToPointer());
	outObj.SetMetaTable(metaTableObj);

	return outObj;
}


void LuaObject::Register(String* funcName, LuaPlusCallback* function)
{
	Push();

	LuaState* state = GetState();
	state->PushString(funcName);

	LuaPlus::LuaObject functionHandleObj = CreateGCHandleObj(state->m_state, function);
	LuaPlus::LuaObject parentObj = CreateGCHandleObj(state->m_state, m_state);
	functionHandleObj.Push();
	parentObj.Push();

	state->m_state->PushCClosure(CallbackDispatcher, 2);

	state->SetTable(-3);
	state->Pop();
}


void LuaObject::Register(String* funcName, LuaPlusOldCallback* function)
{
	Push();

	LuaState* state = GetState();
	state->PushString(funcName);

	LuaPlus::LuaObject functionHandleObj = CreateGCHandleObj(state->m_state, function);
	LuaPlus::LuaObject parentObj = CreateGCHandleObj(state->m_state, m_state);
	functionHandleObj.Push();
	parentObj.Push();

	state->m_state->PushCClosure(OldCallbackDispatcher, 2);

	state->SetTable(-3);
	state->Pop();
}


static int MethodInfoDispatcher(LuaPlus::LuaState* inState)
{
	IntPtr instanceHandle = (IntPtr)LuaPlus::LuaStackObject(inState, lua_upvalueindex(1)).GetUserData();
	IntPtr functionHandle = (IntPtr)LuaPlus::LuaStackObject(inState, lua_upvalueindex(2)).GetUserData();
	IntPtr stateHandle = (IntPtr)LuaPlus::LuaStackObject(inState, lua_upvalueindex(3)).GetUserData();
	Object* instance = __try_cast<Object*>(GCHandle::op_Explicit(instanceHandle).Target);
	MethodInfo* methodInfo = __try_cast<MethodInfo*>(GCHandle::op_Explicit(functionHandle).Target);
	ManagedLuaPlus::LuaState* state = __try_cast<ManagedLuaPlus::LuaState*>(GCHandle::op_Explicit(stateHandle).Target);

	ParameterInfo* parameters[] = methodInfo->GetParameters();

	int numParms = inState->GetTop();
	Object* parms[] = new Object*[numParms];
	for (int i = 1; i <= numParms; ++i)
	{
		LuaPlus::LuaStackObject obj(state->m_state, i);
		if (parameters[i - 1]->ParameterType == __typeof(int))
		{
			if (obj.IsNumber())
				parms[i - 1] = __box(obj.GetInteger());
		}
	}

	methodInfo->Invoke(instance, parms);
	return 0;
}


void LuaObject::Register(String* funcName, Object* instance, MethodInfo* function)
{
	Push();

	LuaState* state = GetState();
	state->PushString(funcName);

	LuaPlus::LuaObject instanceObj = CreateGCHandleObj(state->m_state, instance);
	LuaPlus::LuaObject functionHandleObj = CreateGCHandleObj(state->m_state, function);
	LuaPlus::LuaObject parentObj = CreateGCHandleObj(state->m_state, m_state);
	instanceObj.Push();
	functionHandleObj.Push();
	parentObj.Push();

	state->m_state->PushCClosure(MethodInfoDispatcher, 3);

	state->SetTable(-3);
	state->Pop();
}


static int DelegateDispatcher(LuaPlus::LuaState* inState)
{
	IntPtr functionHandle = (IntPtr)LuaPlus::LuaStackObject(inState, lua_upvalueindex(1)).GetUserData();
	IntPtr stateHandle = (IntPtr)LuaPlus::LuaStackObject(inState, lua_upvalueindex(2)).GetUserData();
	Delegate* theDelegate = __try_cast<Delegate*>(GCHandle::op_Explicit(functionHandle).Target);
	ManagedLuaPlus::LuaState* state = __try_cast<ManagedLuaPlus::LuaState*>(GCHandle::op_Explicit(stateHandle).Target);

	MethodInfo* methodInfo = theDelegate->Method;
	ParameterInfo* parameters[] = methodInfo->GetParameters();

	int numParms = inState->GetTop();
	Object* parms[] = new Object*[numParms];
	for (int i = 1; i <= numParms; ++i)
	{
		LuaPlus::LuaStackObject obj(state->m_state, i);
		if (parameters[i - 1]->ParameterType == __typeof(int))
		{
			if (obj.IsNumber())
				parms[i - 1] = __box(obj.GetInteger());
		}
	}

	methodInfo->Invoke(theDelegate->Target, parms);
	return 0;
}


void LuaObject::Register(String* funcName, Delegate* function)
{
	Push();

	LuaState* state = GetState();
	state->PushString(funcName);

	LuaPlus::LuaObject functionHandleObj = CreateGCHandleObj(state->m_state, function);
	LuaPlus::LuaObject parentObj = CreateGCHandleObj(state->m_state, m_state);
	functionHandleObj.Push();
	parentObj.Push();

	state->m_state->PushCClosure(DelegateDispatcher, 2);

	state->SetTable(-3);
	state->Pop();
}


__gc private class Helper
{
public:
	static void Push(LuaState* state, int arg)
	{
		state->PushNumber(arg);
	}

	static void Push(LuaState* state, String* arg)
	{
		state->PushString(MarshalStringANSI(arg));
	}

	static void Push(LuaState* state, LuaObject* obj)
	{
		state;
		obj->Push();
	}

	static void Push(LuaState* state, LuaStackObject* obj)
	{
		state;
		obj->Push();
	}
};



void LuaObject::Call([ParamArray] Object* args[])
{
	Push();

	System::Type* types[] = new System::Type*[2];
	types[0] = __typeof(ManagedLuaPlus::LuaState);

	Object* objects[] = new Object*[2];
	objects[0] = m_state;

	Helper* helper = new Helper();

	for (int i = 0; i < args->Length; ++i)
	{
		types[1] = args[i]->GetType();
		MethodInfo* pushInfo = __typeof(ManagedLuaPlus::Helper)->GetMethod("Push", types);
		if (!pushInfo)
		{
			String* str = String::Format("Array index {0} is not of a suitable type for pushing to the Lua stack.", __box(i));
			throw new System::ArgumentException(str);
		}

		objects[1] = args[i];
		pushInfo->Invoke(helper, objects);
	}

	LuaState* state = GetState();
	state->m_state->Call(args->Length, 0);
}


String* LuaNode::ToString()
{
	return String::Format("{0} / {1}", m_key->WatchType, m_value->WatchType);
}


LuaState* LuaObject::GetState()
{
	return static_cast<LuaState*>(m_state);
}


LuaObject::~LuaObject()
{
	if (GetState())
		GetState()->m_objects->Remove(this);
	delete m_object;
}

} // namespace ManagedLuaPlus

