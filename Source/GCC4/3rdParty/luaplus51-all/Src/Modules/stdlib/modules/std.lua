-- @module std
-- Lua standard library

-- TODO: Add @class and @meth tags; introduce @meth by replacing @func
--   Foo:bar with @meth bar
-- TODO: Write a style guide (indenting/wrapping, capitalisation,
--   function and variable names); library functions should call
--   error, not die; OO vs non-OO (a thorny problem)
-- TODO: Add tests for each function immediately after the function;
--   this also helps to check module dependencies
-- TODO: precompile and make import check for a .lc version of
--   each file, and load it if it's newer than the .lua version.

module ("std", package.seeall)

require "base"
require "debug_ext"
require "table_ext"
require "list"
require "object"
require "lcs"
require "string_ext"
require "math_ext"
require "io_ext"
require "getopt"
require "set"
