LuaInterface 1.4.0
------------------

Copyright © 2003-2006 Fabio Mascarenhas de Queiroz

Maintainer: Kevin Hester, kevinh@geeksville.com

compat-5.1.lua is Copyright © 2005 Kepler Project

lua50.dll and lua50.exe are Copyright © 2005 Tecgraf, PUC-Rio

Instructions for installing and using in the doc/guide.pdf file.

What's New in LuaInterface 1.4
----------

Note: Fabio area of interest has moved off in other directions (hopefully only temporarily).
I've talked with Fabio and he's said he's okay with me doing a new release with various fixes
I've made over the last few months.  It is possible I've broken something, but I've been using this
within a large CLR project with great success.  Please send bug reports via luaforge.  

Changes since 1.3:

Visual Studio 2005/.Net 2.0 is supported.

Compat-5.1 is modified to expect backslash as the path seperator.

LuaInterface will now work correctly with Generic C# classes.

CLR inner types are now supported.

Fixed a problem where sometimes Lua proxy objects would be associated with the wrong CLR object.

If a CLR class has an array accessor, the elements can be accessed using the regular Lua indexing 
interface.

Add CLRPackage.lua to the samples directory.  This class makes it much easier to automatically 
load referenced assemblies.  In the next release this loading will be automatic.

Various other minor fixes that I've forgotten.  I'll keep better track next time.

Static CLR methods and properties are now accessible from Lua.

Note: LuaInterface is still based on Lua 5.0.2.  If someone really wants us to upgrade to Lua 5.1
please send me a note.

What's New in LuaInterface 1.3
----------

LuaInterface now works with LuaBinaries Release 2 (http://luabinaries.luaforge.net)
and Compat-5.1 Release 3 (http://luaforge.net/projects/compat). The loader DLL is now 
called luanet.dll, and does not need a luainterface.lua file anymore
(just put LuaInterface.dll in the GAC, luanet.dll in your package.cpath, and
do require"luanet").

Fixed a bug in the treatment of the char type (thanks to Ron Scott).

LuaInterface.dll now has a strong name, and can be put in the GAC (thanks to Ivan Voras).

You can now use foreach with instances of LuaTable (thanks to Zachary Landau).

There is an alternate form of loading assemblies and importing types (based on an
anonymous contribution in the Lua wiki). Check the _alt files in the samples folder.


What's New in LuaInterface 1.2.1
--------------------------------

Now checks if two LuaInterface.Lua instances are trying to share the same Lua state,
and throws an exception if this is the case. Also included readonly clauses in public
members of the Lua and ObjectTranslator classes.

This version includes the source of LuaInterfaceLoader.dll, with VS.Net 2003 project
files.

What's New in LuaInterface 1.2
------------------------------

LuaInterface now can be loaded as a module, so you can use the lua standalone
interpreter to run scripts. Thanks to Paul Winwood for this idea and sample code
showing how to load the CLR from a C++ program. The module is "luainterface". Make
sure Lua can find luainterface.lua, and LuaInterfaceLoader.dll is either in the
current directory or the GAC. The samples now load LuaInterface as a module, in
its own namespace.

The get_method_bysig, get_constructor_bysig and make_object were changed: now you
pass the *names* of the types to them, instead of the types themselves. E.g:

  get_method_bysig(obj,"method","System.String")

instead of

  String = import_type("System.String")
  get_method_bysig(obj,"method",String)

Make sure the assemblies of the types you are passing have been loaded, or the call
will fail. The test cases in src/TestLuaInterface/TestLua.cs have examples of the new
functions.
