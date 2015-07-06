namespace LuaInterface.Tests 
{

	using System;
	using LuaInterface;
    using System.Threading;
    using System.Diagnostics;
    using System.Reflection;
	
	/*
	 * Delegates used for testing Lua function -> delegate translation
	 */
	public delegate int TestDelegate1(int a, int b);
	public delegate int TestDelegate2(int a, out int b);
	public delegate void TestDelegate3(int a, ref int b);
	public delegate TestClass TestDelegate4(int a, int b);
	public delegate int TestDelegate5(TestClass a, TestClass b);
	public delegate int TestDelegate6(int a, out TestClass b);
	public delegate void TestDelegate7(int a, ref TestClass b);

	/*
	 * Interface used for testing Lua table -> interface translation
	 */
	public interface ITest 
	{
		int intProp 
		{
			get;
			set;
		}
		TestClass refProp 
		{
			get;
			set;
		}
		int test1(int a,int b);
		int test2(int a, out int b);
		void test3(int a, ref int b);
		TestClass test4(int a, int b);
		int test5(TestClass a, TestClass b);
		int test6(int a, out TestClass b);
		void test7(int a, ref TestClass b);
	}

	public interface IFoo1 
	{
		int foo();
	}

	public interface IFoo2 
	{
		int foo();
	}

    class MyClass
    {
        public int	Func1() { return 1;}
    }


    /// <summary>
    /// test structure passing
    /// </summary>
    public struct TestStruct
    {
        public TestStruct(float val)
        {
            v = val;
        }

        public float v;

        public float val
        {
            get { return v; }
            set { v = value; }
        }
    }


	/*
	 * Sample class used in several test cases to check if
	 * Lua scripts are accessing objects correctly
	 */
	public class TestClass: IFoo1, IFoo2
	{
		public int val;
		private string strVal;
		public TestClass()
		{
			val=0;
		}
		public TestClass(int val) 
		{
			this.val=val;
		}
		public TestClass(string val) 
		{
			this.strVal=val;
		}
		public static TestClass makeFromString(String str) 
		{
			return new TestClass(str);
		}

        bool? nb2 = null;
 
        public bool? NullableBool
        {
            get { return nb2; }
            set { nb2 = value; }
        }


        TestStruct s = new TestStruct();

        public TestStruct Struct
        {
            get { return s; }
            set { s = (TestStruct) value; }
        }

		public int testval 
		{
			get 
			{
				return this.val;
			}
			set
			{
				this.val=value;
			}
		}
		public int this[int index] 
		{
			get { return 1; }
			set { }
		}
		public int this[string index] 
		{
			get { return 1; }
			set { }
		}
		public int sum(int x, int y)
		{
			return x+y;
		}
		public void setVal(int newVal) 
		{
			val=newVal;
		}
		public void setVal(string newVal) 
		{
			strVal=newVal;
		}
		public int getVal() 
		{
			return val;
		}
		public string getStrVal() 
		{
			return strVal;
		}
		public int outVal(out int val) 
		{
			val=5;
			return 3;
		}
		public int outVal(out int val, int val2) 
		{
			val=5;
			return val2;
		}
		public int outVal(int val, ref int val2) 
		{
			val2=val+val2;
			return val;
		}
		public int callDelegate1(TestDelegate1 del) 
		{
			return del(2,3);
		}
		public int callDelegate2(TestDelegate2 del) 
		{
			int a=3;
			int b=del(2,out a);
			return a+b;
		}
		public int callDelegate3(TestDelegate3 del) 
		{
			int a=3;
			del(2,ref a);
			//Console.WriteLine(a);
			return a;
		}
		public int callDelegate4(TestDelegate4 del) 
		{
			return del(2,3).testval;
		}
		public int callDelegate5(TestDelegate5 del) 
		{
			return del(new TestClass(2),new TestClass(3));
		}
		public int callDelegate6(TestDelegate6 del) 
		{
			TestClass test=new TestClass();
			int a=del(2,out test);
			return a+test.testval;
		}
		public int callDelegate7(TestDelegate7 del) 
		{
			TestClass test=new TestClass(3);
			del(2,ref test);
			return test.testval;
		}
		public int callInterface1(ITest itest) 
		{
			return itest.test1(2,3);
		}
		public int callInterface2(ITest itest) 
		{
			int a=3;
			int b=itest.test2(2,out a);
			return a+b;
		}
		public int callInterface3(ITest itest) 
		{
			int a=3;
			itest.test3(2,ref a);
			//Console.WriteLine(a);
			return a;
		}
		public int callInterface4(ITest itest) 
		{
			return itest.test4(2,3).testval;
		}
		public int callInterface5(ITest itest) 
		{
			return itest.test5(new TestClass(2),new TestClass(3));
		}
		public int callInterface6(ITest itest) 
		{
			TestClass test=new TestClass();
			int a=itest.test6(2,out test);
			return a+test.testval;
		}
		public int callInterface7(ITest itest) 
		{
			TestClass test=new TestClass(3);
			itest.test7(2,ref test);
			return test.testval;
		}
		public int callInterface8(ITest itest) 
		{
			itest.intProp=3;
			return itest.intProp;
		}
		public int callInterface9(ITest itest) 
		{
			itest.refProp=new TestClass(3);
			return itest.refProp.testval;
		}
		public void exceptionMethod() 
		{
			throw new Exception("exception test");
		}
		public virtual int overridableMethod(int x,int y) 
		{
			return x+y;
		}
		public static int callOverridable(TestClass test,int x,int y) 
		{
			return test.overridableMethod(x,y);
		}
		int IFoo1.foo() 
		{
			return 3;
		}
		public int foo() 
		{
			return 5;
		}
	}

	/*
	 * Automated test cases for LuaInterface
	 *
	 * Author: Fabio Mascarenhas
	 * Version: 1.0
	 */
	public class TestLuaInterface 
	{
		private Lua lua;
		/*
		 * Executed before each test case
		 */
		public void Init() 
		{
			lua=new Lua();

			GC.Collect();  // runs GC to expose unprotected delegates
		}
		/*
		 * Executed after each test case
		 */
		public void Destroy()
		{
			lua=null;
		}

#if false
        // I've commented out the nunit based tests until they can run standalone - so that users don't need nunit to run TestLua

		/*
		 * Tests if DoString is correctly returning values
		 */
		[Test]
		public void DoString() 
		{
			object[] res=lua.DoString("a=2\nreturn a,3");
			//Console.WriteLine("a="+res[0]+", b="+res[1]);
			Assertion.AssertEquals(res[0],2);
			Assertion.AssertEquals(res[1],3);
		}
		/*
		 * Tests getting of global numeric variables
		 */
		[Test]
		public void GetGlobalNumber() 
		{
			lua.DoString("a=2");
			double num=lua.GetNumber("a");
			//Console.WriteLine("a="+num);
			Assertion.AssertEquals(num,2);
		}
		/*
		 * Tests setting of global numeric variables
		 */
		[Test]
		public void SetGlobalNumber() 
		{
			lua.DoString("a=2");
			lua["a"]=3;
			double num=lua.GetNumber("a");
			//Console.WriteLine("a="+num);
			Assertion.AssertEquals(num,3);
		}
		/*
		 * Tests getting of numeric variables from tables
		 * by specifying variable path
		 */
		[Test]
		public void GetNumberInTable() 
		{
			lua.DoString("a={b={c=2}}");
			double num=lua.GetNumber("a.b.c");
			//Console.WriteLine("a.b.c="+num);
			Assertion.AssertEquals(num,2);
		}
		/*
		 * Tests setting of numeric variables from tables
		 * by specifying variable path
		 */
		[Test]
		public void SetNumberInTable() 
		{
			lua.DoString("a={b={c=2}}");
			lua["a.b.c"]=3;
			double num=lua.GetNumber("a.b.c");
			//Console.WriteLine("a.b.c="+num);
			Assertion.AssertEquals(num,3);
		}
		/*
		 * Tests getting of global string variables
		 */
		[Test]
		public void GetGlobalString() 
		{
			lua.DoString("a=\"test\"");
			string str=lua.GetString("a");
			//Console.WriteLine("a="+str);
			Assertion.AssertEquals(str,"test");
		}
		/*
		 * Tests setting of global string variables
		 */
		[Test]
		public void SetGlobalString() 
		{
			lua.DoString("a=\"test\"");
			lua["a"]="new test";
			string str=lua.GetString("a");
			//Console.WriteLine("a="+str);
			Assertion.AssertEquals(str,"new test");
		}
		/*
		 * Tests getting of string variables from tables
		 * by specifying variable path
		 */
		[Test]
		public void GetStringInTable() 
		{
			lua.DoString("a={b={c=\"test\"}}");
			string str=lua.GetString("a.b.c");
			//Console.WriteLine("a.b.c="+str);
			Assertion.AssertEquals(str,"test");
		}
		/*
		 * Tests setting of string variables from tables
		 * by specifying variable path
		 */
		[Test]
		public void SetStringInTable() 
		{
			lua.DoString("a={b={c=\"test\"}}");
			lua["a.b.c"]="new test";
			string str=lua.GetString("a.b.c");
			//Console.WriteLine("a.b.c="+str);
			Assertion.AssertEquals(str,"new test");
		}
		/*
		 * Tests getting and setting of global table variables
		 */
		[Test]
		public void GetAndSetTable() 
		{
			lua.DoString("a={b={c=2}}\nb={c=3}");
			LuaTable tab=lua.GetTable("b");
			lua["a.b"]=tab;
			double num=lua.GetNumber("a.b.c");
			//Console.WriteLine("a.b.c="+num);
			Assertion.AssertEquals(num,3);
		}
		/*
		 * Tests getting of numeric field of a table
		 */
		[Test]
		public void GetTableNumericField1() 
		{
			lua.DoString("a={b={c=2}}");
			LuaTable tab=lua.GetTable("a.b");
			double num=(double)tab["c"];
			//Console.WriteLine("a.b.c="+num);
			Assertion.AssertEquals(num,2);
		}
		/*
		 * Tests getting of numeric field of a table
		 * (the field is inside a subtable)
		 */
		[Test]
		public void GetTableNumericField2() 
		{
			lua.DoString("a={b={c=2}}");
			LuaTable tab=lua.GetTable("a");
			double num=(double)tab["b.c"];
			//Console.WriteLine("a.b.c="+num);
			Assertion.AssertEquals(num,2);
		}
		/*
		 * Tests setting of numeric field of a table
		 */
		[Test]
		public void SetTableNumericField1() 
		{
			lua.DoString("a={b={c=2}}");
			LuaTable tab=lua.GetTable("a.b");
			tab["c"]=3;
			double num=lua.GetNumber("a.b.c");
			//Console.WriteLine("a.b.c="+num);
			Assertion.AssertEquals(num,3);
		}
		/*
		 * Tests setting of numeric field of a table
		 * (the field is inside a subtable)
		 */
		[Test]
		public void SetTableNumericField2() 
		{
			lua.DoString("a={b={c=2}}");
			LuaTable tab=lua.GetTable("a");
			tab["b.c"]=3;
			double num=lua.GetNumber("a.b.c");
			//Console.WriteLine("a.b.c="+num);
			Assertion.AssertEquals(num,3);
		}
		/*
		 * Tests getting of string field of a table
		 */
		[Test]
		public void GetTableStringField1() 
		{
			lua.DoString("a={b={c=\"test\"}}");
			LuaTable tab=lua.GetTable("a.b");
			string str=(string)tab["c"];
			//Console.WriteLine("a.b.c="+str);
			Assertion.AssertEquals(str,"test");
		}
		/*
		 * Tests getting of string field of a table
		 * (the field is inside a subtable)
		 */
		[Test]
		public void GetTableStringField2() 
		{
			lua.DoString("a={b={c=\"test\"}}");
			LuaTable tab=lua.GetTable("a");
			string str=(string)tab["b.c"];
			//Console.WriteLine("a.b.c="+str);
			Assertion.AssertEquals(str,"test");
		}
		/*
		 * Tests setting of string field of a table
		 */
		[Test]
		public void SetTableStringField1() 
		{
			lua.DoString("a={b={c=\"test\"}}");
			LuaTable tab=lua.GetTable("a.b");
			tab["c"]="new test";
			string str=lua.GetString("a.b.c");
			//Console.WriteLine("a.b.c="+str);
			Assertion.AssertEquals(str,"new test");
		}
		/*
		 * Tests setting of string field of a table
		 * (the field is inside a subtable)
		 */
		[Test]
		public void SetTableStringField2() 
		{
			lua.DoString("a={b={c=\"test\"}}");
			LuaTable tab=lua.GetTable("a");
			tab["b.c"]="new test";
			string str=lua.GetString("a.b.c");
			//Console.WriteLine("a.b.c="+str);
			Assertion.AssertEquals(str,"new test");
		}
		/*
		 * Tests calling of a global function with zero arguments
		 */
		[Test]
		public void CallGlobalFunctionNoArgs() 
		{
			lua.DoString("a=2\nfunction f()\na=3\nend");
			lua.GetFunction("f").Call();
			double num=lua.GetNumber("a");
			//Console.WriteLine("a="+num);
			Assertion.AssertEquals(num,3);
		}
		/*
		 * Tests calling of a global function with one argument
		 */
		[Test]
		public void CallGlobalFunctionOneArg() 
		{
			lua.DoString("a=2\nfunction f(x)\na=a+x\nend");
			lua.GetFunction("f").Call(1);
			double num=lua.GetNumber("a");
			//Console.WriteLine("a="+num);
			Assertion.AssertEquals(num,3);
		}
		/*
		 * Tests calling of a global function with two arguments
		 */
		[Test]
		public void CallGlobalFunctionTwoArgs() 
		{
			lua.DoString("a=2\nfunction f(x,y)\na=x+y\nend");
			lua.GetFunction("f").Call(1,3);
			double num=lua.GetNumber("a");
			//Console.WriteLine("a="+num);
			Assertion.AssertEquals(num,4);
		}
		/*
		 * Tests calling of a global function that returns one value
		 */
		[Test]
		public void CallGlobalFunctionOneReturn() 
		{
			lua.DoString("function f(x)\nreturn x+2\nend");
			object[] ret=lua.GetFunction("f").Call(3);
			//Console.WriteLine("ret="+ret[0]);
			Assertion.AssertEquals(1,ret.Length);
			Assertion.AssertEquals(5,ret[0]);
		}
		/*
		 * Tests calling of a global function that returns two values
		 */
		[Test]
		public void CallGlobalFunctionTwoReturns() 
		{
			lua.DoString("function f(x,y)\nreturn x,x+y\nend");
			object[] ret=lua.GetFunction("f").Call(3,2);
			//Console.WriteLine("ret="+ret[0]+","+ret[1]);
			Assertion.AssertEquals(2,ret.Length);
			Assertion.AssertEquals(3,ret[0]);
			Assertion.AssertEquals(5,ret[1]);
		}
		/*
		 * Tests calling of a function inside a table
		 */
		[Test]
		public void CallTableFunctionTwoReturns() 
		{
			lua.DoString("a={}\nfunction a.f(x,y)\nreturn x,x+y\nend");
			object[] ret=lua.GetFunction("a.f").Call(3,2);
			//Console.WriteLine("ret="+ret[0]+","+ret[1]);
			Assertion.AssertEquals(2,ret.Length);
			Assertion.AssertEquals(3,ret[0]);
			Assertion.AssertEquals(5,ret[1]);
		}
		/*
		 * Tests setting of a global variable to a CLR object value
		 */
		[Test]
		public void SetGlobalObject() 
		{
			TestClass t1=new TestClass();
			t1.testval=4;
			lua["netobj"]=t1;
			object o=lua["netobj"];
			TestClass t2=(TestClass)lua["netobj"];
			Assertion.AssertEquals(t2.testval,4);
			Assertion.Assert(t1==t2);
		}
		/*
		 * Tests if CLR object is being correctly collected by Lua
		 */
		[Test]
		public void GarbageCollection() 
		{
			TestClass t1=new TestClass();
			t1.testval=4;
			lua["netobj"]=t1;
			TestClass t2=(TestClass)lua["netobj"];
			Assertion.Assert(lua.translator.objects[0]!=null);
			lua.DoString("netobj=nil;collectgarbage();");
			Assertion.Assert(lua.translator.objects[0]==null);
		}
		/*
		 * Tests setting of a table field to a CLR object value
		 */
		[Test]
		public void SetTableObjectField1() 
		{
			lua.DoString("a={b={c=\"test\"}}");
			LuaTable tab=lua.GetTable("a.b");
			TestClass t1=new TestClass();
			t1.testval=4;
			tab["c"]=t1;
			TestClass t2=(TestClass)lua["a.b.c"];
			//Console.WriteLine("a.b.c="+t2.testval);
			Assertion.AssertEquals(t2.testval,4);
			Assertion.Assert(t1==t2);
		}
		/*
		 * Tests reading and writing of an object's field
		 */
		[Test]
		public void AccessObjectField() 
		{
			TestClass t1=new TestClass();
			t1.val=4;
			lua["netobj"]=t1;
			lua.DoString("var=netobj.val");
			double var=(double)lua["var"];
			//Console.WriteLine("value from Lua="+var);
			Assertion.AssertEquals(4,var);
			lua.DoString("netobj.val=3");
			Assertion.AssertEquals(3,t1.val);
			//Console.WriteLine("new val (from Lua)="+t1.val);
		}
		/*
		 * Tests reading and writing of an object's non-indexed
		 * property
		 */
		[Test]
		public void AccessObjectProperty() 
		{
			TestClass t1=new TestClass();
			t1.testval=4;
			lua["netobj"]=t1;
			lua.DoString("var=netobj.testval");
			double var=(double)lua["var"];
			//Console.WriteLine("value from Lua="+var);
			Assertion.AssertEquals(4,var);
			lua.DoString("netobj.testval=3");
			Assertion.AssertEquals(3,t1.testval);
			//Console.WriteLine("new val (from Lua)="+t1.testval);
		}
		/*
		 * Tests calling of an object's method with no overloads
		 */
		[Test]
		public void CallObjectMethod() 
		{
			TestClass t1=new TestClass();
			t1.testval=4;
			lua["netobj"]=t1;
			lua.DoString("netobj:setVal(3)");
			Assertion.AssertEquals(3,t1.testval);
			//Console.WriteLine("new val(from C#)="+t1.testval);
			lua.DoString("val=netobj:getVal()");
			int val=(int)lua.GetNumber("val");
			Assertion.AssertEquals(3,val);
			//Console.WriteLine("new val(from Lua)="+val);
		}
		/*
		 * Tests calling of an object's method with overloading
		 */
		[Test]
		public void CallObjectMethodByType() 
		{
			TestClass t1=new TestClass();
			lua["netobj"]=t1;
			lua.DoString("netobj:setVal('str')");
			Assertion.AssertEquals("str",t1.getStrVal());
			//Console.WriteLine("new val(from C#)="+t1.getStrVal());
		}
		/*
		 * Tests calling of an object's method with no overloading
		 * and out parameters
		 */
		[Test]
		public void CallObjectMethodOutParam() 
		{
			TestClass t1=new TestClass();
			lua["netobj"]=t1;
			lua.DoString("a,b=netobj:outVal()");
			int a=(int)lua.GetNumber("a");
			int b=(int)lua.GetNumber("b");
			Assertion.AssertEquals(3,a);
			Assertion.AssertEquals(5,b);
			//Console.WriteLine("function returned (from lua)="+a+","+b);
		}
		/*
		 * Tests calling of an object's method with overloading and
		 * out params
		 */
		[Test]
		public void CallObjectMethodOverloadedOutParam() 
		{
			TestClass t1=new TestClass();
			lua["netobj"]=t1;
			lua.DoString("a,b=netobj:outVal(2)");
			int a=(int)lua.GetNumber("a");
			int b=(int)lua.GetNumber("b");
			Assertion.AssertEquals(2,a);
			Assertion.AssertEquals(5,b);
			//Console.WriteLine("function returned (from lua)="+a+","+b);
		}
		/*
		 * Tests calling of an object's method with ref params
		 */
		[Test]
		public void CallObjectMethodByRefParam() 
		{
			TestClass t1=new TestClass();
			lua["netobj"]=t1;
			lua.DoString("a,b=netobj:outVal(2,3)");
			int a=(int)lua.GetNumber("a");
			int b=(int)lua.GetNumber("b");
			Assertion.AssertEquals(2,a);
			Assertion.AssertEquals(5,b);
			//Console.WriteLine("function returned (from lua)="+a+","+b);
		}
		/*
		 * Tests calling of two versions of an object's method that have
		 * the same name and signature but implement different interfaces
		 */
		[Test]
		public void CallObjectMethodDistinctInterfaces() 
		{
			TestClass t1=new TestClass();
			lua["netobj"]=t1;
			lua.DoString("a=netobj:foo()");
			lua.DoString("b=netobj['LuaInterface.Tests.IFoo1.foo'](netobj)");
			int a=(int)lua.GetNumber("a");
			int b=(int)lua.GetNumber("b");
			Assertion.AssertEquals(5,a);
			Assertion.AssertEquals(3,b);
			//Console.WriteLine("function returned (from lua)="+a+","+b);
		}
		/*
		 * Tests instantiating an object with no-argument constructor
		 */
		[Test]
		public void CreateNetObjectNoArgsCons()
		{
			lua.DoString("load_assembly(\"TestLua\")");
			lua.DoString("TestClass=import_type(\"LuaInterface.Tests.TestClass\")");
			lua.DoString("test=TestClass()");
			lua.DoString("test:setVal(3)");
			object[] res=lua.DoString("return test");
			TestClass test=(TestClass)res[0];
			//Console.WriteLine("returned: "+test.testval);
			Assertion.AssertEquals(3,test.testval);
		}
		/*
		 * Tests instantiating an object with one-argument constructor
		 */
		[Test]
		public void CreateNetObjectOneArgCons()
		{
			lua.DoString("load_assembly(\"TestLua\")");
			lua.DoString("TestClass=import_type(\"LuaInterface.Tests.TestClass\")");
			lua.DoString("test=TestClass(3)");
			object[] res=lua.DoString("return test");
			TestClass test=(TestClass)res[0];
			//Console.WriteLine("returned: "+test.testval);
			Assertion.AssertEquals(3,test.testval);
		}
		/*
		 * Tests instantiating an object with overloaded constructor
		 */
		[Test]
		public void CreateNetObjectOverloadedCons()
		{
			lua.DoString("load_assembly(\"TestLua\")");
			lua.DoString("TestClass=import_type(\"LuaInterface.Tests.TestClass\")");
			lua.DoString("test=TestClass('str')");
			object[] res=lua.DoString("return test");
			TestClass test=(TestClass)res[0];
			//Console.WriteLine("returned: "+test.getStrVal());
			Assertion.AssertEquals("str",test.getStrVal());
		}
		/*
		 * Tests getting item of a CLR array
		 */
		[Test]
		public void ReadArrayField() 
		{
			string[] arr=new string[] { "str1", "str2", "str3" };
			lua["netobj"]=arr;
			lua.DoString("val=netobj[1]");
			string val=lua.GetString("val");
			Assertion.AssertEquals("str2",val);
			//Console.WriteLine("new val(from array to Lua)="+val);
		}
		/*
		 * Tests setting item of a CLR array
		 */
		[Test]
		public void WriteArrayField() 
		{
			string[] arr=new string[] { "str1", "str2", "str3" };
			lua["netobj"]=arr;
			lua.DoString("netobj[1]='test'");
			Assertion.AssertEquals("test",arr[1]);
			//Console.WriteLine("new val(from Lua to array)="+arr[1]);
		}
		/*
		 * Tests creating a new CLR array
		 */
		[Test]
		public void CreateArray() 
		{
			lua.DoString("load_assembly(\"TestLua\")");
			lua.DoString("TestClass=import_type(\"LuaInterface.Tests.TestClass\")");
			lua.DoString("arr=TestClass[3]");
			lua.DoString("for i=0,2 do arr[i]=TestClass(i+1) end");
			TestClass[] arr=(TestClass[])lua["arr"];
			Assertion.AssertEquals(arr[1].testval,2);
		}
		/*
		 * Tests passing a Lua function to a delegate
		 * with value-type arguments
		 */
		[Test]
		public void LuaDelegateValueTypes() 
		{
			lua.DoString("load_assembly('TestLua')");
			lua.DoString("TestClass=import_type('LuaInterface.Tests.TestClass')");
			lua.DoString("test=TestClass()");
			lua.DoString("function func(x,y) return x+y; end");
			lua.DoString("test=TestClass()");
			lua.DoString("a=test:callDelegate1(func)");
			int a=(int)lua.GetNumber("a");
			Assertion.AssertEquals(5,a);
			//Console.WriteLine("delegate returned: "+a);
		}
		/*
		 * Tests passing a Lua function to a delegate
		 * with value-type arguments and out params
		 */
		[Test]
		public void LuaDelegateValueTypesOutParam() 
		{
			lua.DoString("load_assembly('TestLua')");
			lua.DoString("TestClass=import_type('LuaInterface.Tests.TestClass')");
			lua.DoString("test=TestClass()");
			lua.DoString("function func(x) return x,x*2; end");
			lua.DoString("test=TestClass()");
			lua.DoString("a=test:callDelegate2(func)");
			int a=(int)lua.GetNumber("a");
			Assertion.AssertEquals(6,a);
			//Console.WriteLine("delegate returned: "+a);
		}
		/*
		 * Tests passing a Lua function to a delegate
		 * with value-type arguments and ref params
		 */
		[Test]
		public void LuaDelegateValueTypesByRefParam() 
		{
			lua.DoString("load_assembly('TestLua')");
			lua.DoString("TestClass=import_type('LuaInterface.Tests.TestClass')");
			lua.DoString("test=TestClass()");
			lua.DoString("function func(x,y) return x+y; end");
			lua.DoString("test=TestClass()");
			lua.DoString("a=test:callDelegate3(func)");
			int a=(int)lua.GetNumber("a");
			Assertion.AssertEquals(5,a);
			//Console.WriteLine("delegate returned: "+a);
		}
		/*
		 * Tests passing a Lua function to a delegate
		 * with value-type arguments that returns a reference type
		 */
		[Test]
		public void LuaDelegateValueTypesReturnReferenceType() 
		{
			lua.DoString("load_assembly('TestLua')");
			lua.DoString("TestClass=import_type('LuaInterface.Tests.TestClass')");
			lua.DoString("test=TestClass()");
			lua.DoString("function func(x,y) return TestClass(x+y); end");
			lua.DoString("test=TestClass()");
			lua.DoString("a=test:callDelegate4(func)");
			int a=(int)lua.GetNumber("a");
			Assertion.AssertEquals(5,a);
			//Console.WriteLine("delegate returned: "+a);
		}
		/*
		 * Tests passing a Lua function to a delegate
		 * with reference type arguments
		 */
		[Test]
		public void LuaDelegateReferenceTypes() 
		{
			lua.DoString("load_assembly('TestLua')");
			lua.DoString("TestClass=import_type('LuaInterface.Tests.TestClass')");
			lua.DoString("test=TestClass()");
			lua.DoString("function func(x,y) return x.testval+y.testval; end");
			lua.DoString("test=TestClass()");
			lua.DoString("a=test:callDelegate5(func)");
			int a=(int)lua.GetNumber("a");
			Assertion.AssertEquals(5,a);
			//Console.WriteLine("delegate returned: "+a);
		}
		/*
		 * Tests passing a Lua function to a delegate
		 * with reference type arguments and an out param
		 */
		[Test]
		public void LuaDelegateReferenceTypesOutParam() 
		{
			lua.DoString("load_assembly('TestLua')");
			lua.DoString("TestClass=import_type('LuaInterface.Tests.TestClass')");
			lua.DoString("test=TestClass()");
			lua.DoString("function func(x) return x,TestClass(x*2); end");
			lua.DoString("test=TestClass()");
			lua.DoString("a=test:callDelegate6(func)");
			int a=(int)lua.GetNumber("a");
			Assertion.AssertEquals(6,a);
			//Console.WriteLine("delegate returned: "+a);
		}
		/*
		 * Tests passing a Lua function to a delegate
		 * with reference type arguments and a ref param
		 */
		[Test]
		public void LuaDelegateReferenceTypesByRefParam() 
		{
			lua.DoString("load_assembly('TestLua')");
			lua.DoString("TestClass=import_type('LuaInterface.Tests.TestClass')");
			lua.DoString("test=TestClass()");
			lua.DoString("function func(x,y) return TestClass(x+y.testval); end");
			lua.DoString("a=test:callDelegate7(func)");
			int a=(int)lua.GetNumber("a");
			Assertion.AssertEquals(5,a);
			//Console.WriteLine("delegate returned: "+a);
		}
		/*
		 * Tests passing a Lua table as an interface and
		 * calling one of its methods with value-type params
		 */
		[Test]
		public void LuaInterfaceValueTypes() 
		{
			lua.DoString("load_assembly('TestLua')");
			lua.DoString("TestClass=import_type('LuaInterface.Tests.TestClass')");
			lua.DoString("test=TestClass()");
			lua.DoString("itest={}");
			lua.DoString("function itest:test1(x,y) return x+y; end");
			lua.DoString("test=TestClass()");
			lua.DoString("a=test:callInterface1(itest)");
			int a=(int)lua.GetNumber("a");
			Assertion.AssertEquals(5,a);
			//Console.WriteLine("interface returned: "+a);
		}
		/*
		 * Tests passing a Lua table as an interface and
		 * calling one of its methods with value-type params
		 * and an out param
		 */
		[Test]
		public void LuaInterfaceValueTypesOutParam() 
		{
			lua.DoString("load_assembly('TestLua')");
			lua.DoString("TestClass=import_type('LuaInterface.Tests.TestClass')");
			lua.DoString("test=TestClass()");
			lua.DoString("itest={}");
			lua.DoString("function itest:test2(x) return x,x*2; end");
			lua.DoString("test=TestClass()");
			lua.DoString("a=test:callInterface2(itest)");
			int a=(int)lua.GetNumber("a");
			Assertion.AssertEquals(6,a);
			//Console.WriteLine("interface returned: "+a);
		}
		/*
		 * Tests passing a Lua table as an interface and
		 * calling one of its methods with value-type params
		 * and a ref param
		 */
		[Test]
		public void LuaInterfaceValueTypesByRefParam() 
		{
			lua.DoString("load_assembly('TestLua')");
			lua.DoString("TestClass=import_type('LuaInterface.Tests.TestClass')");
			lua.DoString("test=TestClass()");
			lua.DoString("itest={}");
			lua.DoString("function itest:test3(x,y) return x+y; end");
			lua.DoString("test=TestClass()");
			lua.DoString("a=test:callInterface3(itest)");
			int a=(int)lua.GetNumber("a");
			Assertion.AssertEquals(5,a);
			//Console.WriteLine("interface returned: "+a);
		}
		/*
		 * Tests passing a Lua table as an interface and
		 * calling one of its methods with value-type params
		 * returning a reference type param
		 */
		[Test]
		public void LuaInterfaceValueTypesReturnReferenceType() 
		{
			lua.DoString("load_assembly('TestLua')");
			lua.DoString("TestClass=import_type('LuaInterface.Tests.TestClass')");
			lua.DoString("test=TestClass()");
			lua.DoString("itest={}");
			lua.DoString("function itest:test4(x,y) return TestClass(x+y); end");
			lua.DoString("test=TestClass()");
			lua.DoString("a=test:callInterface4(itest)");
			int a=(int)lua.GetNumber("a");
			Assertion.AssertEquals(5,a);
			//Console.WriteLine("interface returned: "+a);
		}
		/*
		 * Tests passing a Lua table as an interface and
		 * calling one of its methods with reference type params
		 */
		[Test]
		public void LuaInterfaceReferenceTypes() 
		{
			lua.DoString("load_assembly('TestLua')");
			lua.DoString("TestClass=import_type('LuaInterface.Tests.TestClass')");
			lua.DoString("test=TestClass()");
			lua.DoString("itest={}");
			lua.DoString("function itest:test5(x,y) return x.testval+y.testval; end");
			lua.DoString("test=TestClass()");
			lua.DoString("a=test:callInterface5(itest)");
			int a=(int)lua.GetNumber("a");
			Assertion.AssertEquals(5,a);
			//Console.WriteLine("interface returned: "+a);
		}
		/*
		 * Tests passing a Lua table as an interface and
		 * calling one of its methods with reference type params
		 * and an out param
		 */
		[Test]
		public void LuaInterfaceReferenceTypesOutParam() 
		{
			lua.DoString("load_assembly('TestLua')");
			lua.DoString("TestClass=import_type('LuaInterface.Tests.TestClass')");
			lua.DoString("test=TestClass()");
			lua.DoString("itest={}");
			lua.DoString("function itest:test6(x) return x,TestClass(x*2); end");
			lua.DoString("test=TestClass()");
			lua.DoString("a=test:callInterface6(itest)");
			int a=(int)lua.GetNumber("a");
			Assertion.AssertEquals(6,a);
			//Console.WriteLine("interface returned: "+a);
		}
		/*
		 * Tests passing a Lua table as an interface and
		 * calling one of its methods with reference type params
		 * and a ref param
		 */
		[Test]
		public void LuaInterfaceReferenceTypesByRefParam() 
		{
			lua.DoString("load_assembly('TestLua')");
			lua.DoString("TestClass=import_type('LuaInterface.Tests.TestClass')");
			lua.DoString("test=TestClass()");
			lua.DoString("itest={}");
			lua.DoString("function itest:test7(x,y) return TestClass(x+y.testval); end");
			lua.DoString("a=test:callInterface7(itest)");
			int a=(int)lua.GetNumber("a");
			Assertion.AssertEquals(5,a);
			//Console.WriteLine("interface returned: "+a);
		}
		/*
		 * Tests passing a Lua table as an interface and
		 * accessing one of its value-type properties
		 */
		[Test]
		public void LuaInterfaceValueProperty() 
		{
			lua.DoString("load_assembly('TestLua')");
			lua.DoString("TestClass=import_type('LuaInterface.Tests.TestClass')");
			lua.DoString("test=TestClass()");
			lua.DoString("itest={}");
			lua.DoString("function itest:get_intProp() return itest.int_prop; end");
			lua.DoString("function itest:set_intProp(val) itest.int_prop=val; end");
			lua.DoString("a=test:callInterface8(itest)");
			int a=(int)lua.GetNumber("a");
			Assertion.AssertEquals(3,a);
			//Console.WriteLine("interface returned: "+a);
		}
		/*
		 * Tests passing a Lua table as an interface and
		 * accessing one of its reference type properties
		 */
		[Test]
		public void LuaInterfaceReferenceProperty() 
		{
			lua.DoString("load_assembly('TestLua')");
			lua.DoString("TestClass=import_type('LuaInterface.Tests.TestClass')");
			lua.DoString("test=TestClass()");
			lua.DoString("itest={}");
			lua.DoString("function itest:get_refProp() return TestClass(itest.int_prop); end");
			lua.DoString("function itest:set_refProp(val) itest.int_prop=val.testval; end");
			lua.DoString("a=test:callInterface9(itest)");
			int a=(int)lua.GetNumber("a");
			Assertion.AssertEquals(3,a);
			//Console.WriteLine("interface returned: "+a);
		}


		/*
		 * Tests making an object from a Lua table and calling the base
		 * class version of one of the methods the table overrides.
		 */
		[Test]
		public void LuaTableBaseMethod() 
		{
			lua.DoString("load_assembly('TestLua')");
			lua.DoString("TestClass=import_type('LuaInterface.Tests.TestClass')");
			lua.DoString("test={}");
			lua.DoString("function test:overridableMethod(x,y) return 2*self.base:overridableMethod(x,y); end");
			lua.DoString("make_object(test,'LuaInterface.Tests.TestClass')");
			lua.DoString("a=TestClass:callOverridable(test,2,3)");
			int a=(int)lua.GetNumber("a");
			lua.DoString("free_object(test)");
			Assertion.AssertEquals(10,a);
			//Console.WriteLine("interface returned: "+a);
		}
		/*
		 * Tests getting an object's method by its signature
		 * (from object)
		 */
		[Test]
		public void GetMethodBySignatureFromObj() 
		{
			lua.DoString("load_assembly('mscorlib')");
			lua.DoString("load_assembly('TestLua')");
			lua.DoString("TestClass=import_type('LuaInterface.Tests.TestClass')");
			lua.DoString("test=TestClass()");
			lua.DoString("setMethod=get_method_bysig(test,'setVal','System.String')");
			lua.DoString("setMethod('test')");
			TestClass test=(TestClass)lua["test"];
			Assertion.AssertEquals("test",test.getStrVal());
			//Console.WriteLine("interface returned: "+test.getStrVal());
		}
		/*
		 * Tests getting an object's method by its signature
		 * (from type)
		 */
		[Test]
		public void GetMethodBySignatureFromType() 
		{
			lua.DoString("load_assembly('mscorlib')");
			lua.DoString("load_assembly('TestLua')");
			lua.DoString("TestClass=import_type('LuaInterface.Tests.TestClass')");
			lua.DoString("test=TestClass()");
			lua.DoString("setMethod=get_method_bysig(TestClass,'setVal','System.String')");
			lua.DoString("setMethod(test,'test')");
			TestClass test=(TestClass)lua["test"];
			Assertion.AssertEquals("test",test.getStrVal());
			//Console.WriteLine("interface returned: "+test.getStrVal());
		}
		/*
		 * Tests getting a type's method by its signature
		 */
		[Test]
		public void GetStaticMethodBySignature() 
		{
			lua.DoString("load_assembly('mscorlib')");
			lua.DoString("load_assembly('TestLua')");
			lua.DoString("TestClass=import_type('LuaInterface.Tests.TestClass')");
			lua.DoString("make_method=get_method_bysig(TestClass,'makeFromString','System.String')");
			lua.DoString("test=make_method('test')");
			TestClass test=(TestClass)lua["test"];
			Assertion.AssertEquals("test",test.getStrVal());
			//Console.WriteLine("interface returned: "+test.getStrVal());
		}
		/*
		 * Tests getting an object's constructor by its signature
		 */
		[Test]
		public void GetConstructorBySignature() 
		{
			lua.DoString("load_assembly('mscorlib')");
			lua.DoString("load_assembly('TestLua')");
			lua.DoString("TestClass=import_type('LuaInterface.Tests.TestClass')");
			lua.DoString("test_cons=get_constructor_bysig(TestClass,'System.String')");
			lua.DoString("test=test_cons('test')");
			TestClass test=(TestClass)lua["test"];
			Assertion.AssertEquals("test",test.getStrVal());
			//Console.WriteLine("interface returned: "+test.getStrVal());
		}
#endif

        void TestOk(bool flag)
        {
            if (flag)
                Console.WriteLine("Test Passed.");
            else
                Console.WriteLine("Test Failed!!!!");
        }


		/*
		 * Tests capturing an exception
		 */
		public void ThrowException() 
		{
            Init();

            lua.DoString("luanet.load_assembly('mscorlib')");
            lua.DoString("luanet.load_assembly('TestLua')");
            lua.DoString("TestClass=luanet.import_type('LuaInterface.Tests.TestClass')");
			lua.DoString("test=TestClass()");
			lua.DoString("err,errMsg=pcall(test.exceptionMethod,test)");
			bool err=(bool)lua["err"];
			Exception errMsg=(Exception)lua["errMsg"];
            TestOk(!err);
            TestOk("exception test" == errMsg.Message);
			//Console.WriteLine("interface returned: "+errMsg.ToString());

            Destroy();
		}

        /*
         * Tests capturing an exception
         */
        public void ThrowUncaughtException()
        {
            Init();

            lua.DoString("luanet.load_assembly('mscorlib')");
            lua.DoString("luanet.load_assembly('TestLua')");
            lua.DoString("TestClass=luanet.import_type('LuaInterface.Tests.TestClass')");
            lua.DoString("test=TestClass()");

            try
            {
                lua.DoString("test:exceptionMethod()");

                Console.WriteLine("Test failed!!! Should have thrown an exception all the way out of Lua");
            }
            catch (Exception e)
            {
                Console.WriteLine("Uncaught exception success");
            }

            Destroy();
        }


        /*
         * Tests nullable fields
         */
        public void TestNullable()
        {
            Init();

            lua.DoString("luanet.load_assembly('mscorlib')");
            lua.DoString("luanet.load_assembly('TestLua')");
            lua.DoString("TestClass=luanet.import_type('LuaInterface.Tests.TestClass')");
            lua.DoString("test=TestClass()");

            lua.DoString("val=test.NullableBool");
            TestOk(((object)lua["val"]) == null);
            lua.DoString("test.NullableBool = true");
            lua.DoString("val=test.NullableBool");
            TestOk(((bool)lua["val"]) == true);

            Destroy();
        }


        /*
         * Tests structure assignment
         */
        public void TestStructs()
        {
            Init();

            lua.DoString("luanet.load_assembly('TestLua')");
            lua.DoString("TestClass=luanet.import_type('LuaInterface.Tests.TestClass')");
            lua.DoString("test=TestClass()");
            lua.DoString("TestStruct=luanet.import_type('LuaInterface.Tests.TestStruct')");

            lua.DoString("struct=TestStruct(2)");
            lua.DoString("test.Struct = struct");
            lua.DoString("val=test.Struct.val");
            TestOk(((double)lua["val"]) == 2.0);

            Destroy();
        }


        /*
         * Tests functions
         */
        public void TestFunctions()
        {
            Init();

            lua.DoString("luanet.load_assembly('mscorlib')");
            lua.DoString("luanet.load_assembly('TestLua')");
            lua.RegisterFunction("p", null, typeof(System.Console).GetMethod("WriteLine", new Type[] {typeof(String)}));

            /// Lua command that works (prints to console)
            lua.DoString("p('Foo')");

            /// Yet this works...
            lua.DoString("string.gsub('some string', '(%w+)', function(s) p(s) end)");

            /// This fails if you don't fix Lua5.1 lstrlib.c/add_value to treat LUA_TUSERDATA the same as LUA_FUNCTION
            lua.DoString("string.gsub('some string', '(%w+)', p)");

            Destroy();
        }




        /*
         * Tests making an object from a Lua table and calling one of
         * methods the table overrides.
         */
        public void LuaTableOverridedMethod()
        {
            Init();

            lua.DoString("luanet.load_assembly('TestLua')");
            lua.DoString("TestClass=luanet.import_type('LuaInterface.Tests.TestClass')");
            lua.DoString("test={}");
            lua.DoString("function test:overridableMethod(x,y) return x*y; end");
            lua.DoString("luanet.make_object(test,'LuaInterface.Tests.TestClass')");
            lua.DoString("a=TestClass.callOverridable(test,2,3)");
            int a = (int)lua.GetNumber("a");
            lua.DoString("luanet.free_object(test)");
            TestOk(6 == a);
            //Console.WriteLine("interface returned: "+a);
        }


        /*
         * Tests making an object from a Lua table and calling a method
         * the table does not override.
         */
        public void LuaTableInheritedMethod()
        {
            Init();

            lua.DoString("luanet.load_assembly('TestLua')");
            lua.DoString("TestClass=luanet.import_type('LuaInterface.Tests.TestClass')");
            lua.DoString("test={}");
            lua.DoString("function test:overridableMethod(x,y) return x*y; end");
            lua.DoString("luanet.make_object(test,'LuaInterface.Tests.TestClass')");
            lua.DoString("test:setVal(3)");
            lua.DoString("a=test.testval");
            int a = (int)lua.GetNumber("a");
            lua.DoString("luanet.free_object(test)");
            TestOk(3 == a);
            //Console.WriteLine("interface returned: "+a);
        }


        /// <summary>
        /// Basic multiply method which expects 2 floats
        /// </summary>
        /// <param name="val"></param>
        /// <param name="val2"></param>
        /// <returns></returns>
        private float _TestException(float val, float val2)
        {
            return val * val2;
        }


        public void TestEventException()
        {
            Init();

            //Register a C# function
            MethodInfo testException = this.GetType().GetMethod("_TestException", BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.DeclaredOnly | BindingFlags.Instance, null, new Type[] { typeof(float), typeof(float) }, null);
            lua.RegisterFunction("Multiply", this, testException);

            //create the lua event handler code for the entity
            //includes the bad code!
            lua.DoString("function OnClick(sender, eventArgs)\r\n" +
                          "--Multiply expects 2 floats, but instead receives 2 strings\r\n" +
                          "Multiply(asd, we)\r\n" +
                        "end");

            //create the lua event handler code for the entity
            //good code
            //lua.DoString("function OnClick(sender, eventArgs)\r\n" +
            //              "--Multiply expects 2 floats\r\n" +
            //              "Multiply(2, 50)\r\n" +
            //            "end");

            //Create the event handler script
            lua.DoString("function SubscribeEntity(e)\r\ne.Clicked:Add(OnClick)\r\nend");

            //Create the entity object
            Entity entity = new Entity();

            //Register the entity object with the event handler inside lua
            LuaFunction lf = lua.GetFunction("SubscribeEntity");
            lf.Call(new object[1] { entity });

            try
            {
                //Cause the event to be fired
                entity.Click();

                Console.WriteLine("Test failed!!! Should have thrown an exception all the way out of Lua");
            }
            catch (LuaException e)
            {
                Console.WriteLine("Event exception success");
            }

        }


		public static int func(int x, int y) 
		{
			return x+y;
		}
        public int funcInstance(int x,int y)
        {
            return x+y;
        }


        public void RegisterFunctionStressTest()
        {
            LuaFunction fc = null;
            const int Count = 200;  // it seems to work with 41

            Init();

            MyClass t = new MyClass();

            for (int i = 1; i < Count - 1; ++i)
            {
                fc = lua.RegisterFunction("func" + i, t, typeof(MyClass).GetMethod("Func1"));
            }
            fc = lua.RegisterFunction("func" + (Count - 1), t, typeof(MyClass).GetMethod("Func1"));

            lua.DoString("print(func1())");
        }


		/*
		 * Sample test script that shows some of the capabilities of
		 * LuaInterface
		 */
		public static void Main() 
		{
			Console.WriteLine("Starting interpreter...");
			Lua l=new Lua();

			Console.WriteLine("Reading test.lua file...");
			l.DoFile("test.lua");
			double width=l.GetNumber("width");
			double height=l.GetNumber("height");
			string message=l.GetString("message");
			double color_r=l.GetNumber("color.r");
			double color_g=l.GetNumber("color.g");
			double color_b=l.GetNumber("color.b");
			Console.WriteLine("Printing values of global variables width, height and message...");
			Console.WriteLine("width: "+width);
			Console.WriteLine("height: "+height);
			Console.WriteLine("message: "+message);
			Console.WriteLine("Printing values of the 'color' table's fields...");
			Console.WriteLine("color.r: "+color_r);
			Console.WriteLine("color.g: "+color_g);
			Console.WriteLine("color.b: "+color_b);
			width=150;
			Console.WriteLine("Changing width's value and calling Lua function print to show it...");
			l["width"]=width;
			l.GetFunction("print").Call(width);
			message="LuaNet Interface Test";
			Console.WriteLine("Changing message's value and calling Lua function print to show it...");
			l["message"]=message;
			l.GetFunction("print").Call(message);
			color_r=30;
			color_g=10;
			color_b=200;
			Console.WriteLine("Changing color's fields' values and calling Lua function print to show it...");
			l["color.r"]=color_r;
			l["color.g"]=color_g;
			l["color.b"]=color_b;
			l.DoString("print(color.r,color.g,color.b)");
			Console.WriteLine("Printing values of the tree table's fields...");
			double leaf1=l.GetNumber("tree.branch1.leaf1");
			string leaf2=l.GetString("tree.branch1.leaf2");
			string leaf3=l.GetString("tree.leaf3");
			Console.WriteLine("leaf1: "+leaf1);
			Console.WriteLine("leaf2: "+leaf2);
			Console.WriteLine("leaf3: "+leaf3);
			leaf1=30; leaf2="new leaf2";
			Console.WriteLine("Changing tree's fields' values and calling Lua function print to show it...");
			l["tree.branch1.leaf1"]=leaf1; l["tree.branch1.leaf2"]=leaf2;
			l.DoString("print(tree.branch1.leaf1,tree.branch1.leaf2)");
			Console.WriteLine("Returning values from Lua with 'return'...");
			object[] vals=l.DoString("return 2,3");
			Console.WriteLine("Returned: "+vals[0]+" and "+vals[1]);
			Console.WriteLine("Calling a Lua function that returns multiple values...");
			object[] vals1=l.GetFunction("func").Call(2,3);
			Console.WriteLine("Returned: "+vals1[0]+" and "+vals1[1]);
			Console.WriteLine("Creating a table and filling it from C#...");
			l.NewTable("tab");
			l.NewTable("tab.tab");
			l["tab.a"]="a!";
			l["tab.b"]=5.5;
			l["tab.tab.c"]=6.5;
			l.DoString("print(tab.a,tab.b,tab.tab.c)");
			Console.WriteLine("Setting a table as another table's field...");
			l["tab.a"]=l["tab.tab"];
			l.DoString("print(tab.a.c)");
			Console.WriteLine("Registering a C# static method and calling it from Lua...");

            // Pause so we can connect with the debugger
            // Thread.Sleep(30000);

			l.RegisterFunction("func1",null,typeof(TestLuaInterface).GetMethod("func"));
			vals1=l.GetFunction("func1").Call(2,3);
			Console.WriteLine("Returned: "+vals1[0]); 
            TestLuaInterface obj=new TestLuaInterface();
			Console.WriteLine("Registering a C# instance method and calling it from Lua...");
			l.RegisterFunction("func2",obj,typeof(TestLuaInterface).GetMethod("funcInstance"));
			vals1=l.GetFunction("func2").Call(2,3);
			Console.WriteLine("Returned: "+vals1[0]);

            Console.WriteLine("Testing throwing an exception...");
            obj.ThrowUncaughtException();

            Console.WriteLine("Testing catching an exception...");
            obj.ThrowException();

            Console.WriteLine("Testing inheriting a method from Lua...");
            obj.LuaTableInheritedMethod();

            Console.WriteLine("Testing overriding a C# method with Lua...");
            obj.LuaTableOverridedMethod(); 

            Console.WriteLine("Stress test RegisterFunction (based on a reported bug)..");
            obj.RegisterFunctionStressTest();

            Console.WriteLine("Test structures...");
            obj.TestStructs();
 
            Console.WriteLine("Test Nullable types...");
            obj.TestNullable();

            Console.WriteLine("Test functions...");
            obj.TestFunctions();

            Console.WriteLine("Test event exceptions...");
            obj.TestEventException();
		}
	}

}