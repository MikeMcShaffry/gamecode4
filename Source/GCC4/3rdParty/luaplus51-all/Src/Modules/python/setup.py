#!/usr/bin/python
from distutils.core import setup, Extension
from distutils.sysconfig import get_python_lib, get_python_version
import os

if os.path.isfile("MANIFEST"):
    os.unlink("MANIFEST")

# You may have to change these
PYLIBS = ["python"+get_python_version(), "pthread", "util"]
PYLIBDIR = [get_python_lib(standard_lib=True)+"/config"]
LUALIBS = ["lua", "lualib"]
LUALIBDIR = []

setup(name="lunatic-python",
      version = "1.0",
      description = "Two-way bridge between Python and Lua",
      author = "Gustavo Niemeyer",
      author_email = "gustavo@niemeyer.net",
      url = "http://labix.org/lunatic-python",
      license = "LGPL",
      long_description =
"""\
Lunatic Python is a two-way bridge between Python and Lua, allowing these
languages to intercommunicate. Being two-way means that it allows Lua inside
Python, Python inside Lua, Lua inside Python inside Lua, Python inside Lua
inside Python, and so on.
""",
      ext_modules = [
                     Extension("lua-python",
                               ["src/pythoninlua.c", "src/luainpython.c"],
                               library_dirs=PYLIBDIR,
                               libraries=PYLIBS,
                               extra_compile_args=["-rdynamic"],
                               extra_link_args=["-rdynamic"]),
                     Extension("lua",
                               ["src/pythoninlua.c", "src/luainpython.c"],
                               library_dirs=LUALIBDIR,
                               libraries=LUALIBS,
                               extra_compile_args=["-rdynamic"],
                               extra_link_args=["-rdynamic"]),
                    ],
      )
