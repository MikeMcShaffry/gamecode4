lqt is still in early development the version here is not complete or
stable and depends on GCC-XML which is rather old software with some
serious bugs. If you want lqt to "just work" I recommend to wait for
the new version in development.
 
 If you want to use it _right_now_, you can still use the gcc-xml version:
  - install gccxml in the path. (f you can't you are out of luck, this
 is why I want to eliminate it)
  - clone the repo
  - execute "lua generate_function.lua"
  - edit qt_generator.lua
  - include in the string at the end all the (qt) classes you need
  - execute it (lua qt_generator.lua)
  - if it does not fail (it invokes gccxml, so it could) it should have
 created the sources of the bindings in the src directory
  - if it fails could be because gccxml has been given wrong paths
 (qt_generator.lua at line 114)
 
 Other steps depend on what you want to do with the bindings. If you
 want to use it as a module in a stand-alone lua, you can just use
 cmake with the provided CMakeLists.txt, place the shared object in the
 lua cpath and require it.
 
 if you want bindings in your application just link to them and call
 the initialization function of each of them on each of the lua_States
 you have.
 
 each file is completely independent from others and binds one single
 qt class. you can bind them indipendently even in the same inheritance
 tree (you can bind QPushButton and not QWidget). everything works
 anyway.
 
 *common.* files are required for each of the bindings.
 *utils.* are used to export some functionality to lua (such as
 ownership of objects, which must be taken care manually)
 *function.* is a dirty hack to make it possible to connect a signal to
 a lua function.
 
