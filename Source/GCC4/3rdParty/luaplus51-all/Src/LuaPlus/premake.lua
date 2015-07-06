------------------------------------------------------------------------------
-- helper function to exclude unneeded files from sources list              --
------------------------------------------------------------------------------

function filter_filelist(tbl, badname)
	local tmp = {}
	for k = 1, table.getn(tbl) do
		if (type(tbl[k]) == "table") then
			tinsert(tmp, filter_filelist(tbl[k], badname))
		elseif ((type(tbl[k]) == "string") and (not string.find(tbl[k], badname))) then
			tinsert(tmp, tbl[k])
		end
	end
	return tmp
end

------------------------------------------------------------------------------
-- lua library package                                                      --
------------------------------------------------------------------------------

package = newpackage()
package.name = "lualib"
package.kind = "lib"
package.language = "c++"

package.includepaths = { "include", "src", "." }
package.defines = { "LUA_OPNAMES", "WIN32", "_LIB", "NDEBUG", "LUAPLUS_LIB" }

package.files = { matchfiles("src/*.c", "src/*.h", "src/lib/*.c", "include/*.h", "*.c", "*.cpp", "*.h") }
-- exclude luace.h, luace.c and luiolib.c from the list of sources
package.files = filter_filelist(package.files, "luiolib")
package.files = filter_filelist(package.files, "luace")

------------------------------------------------------------------------------
-- lua interpreter package                                                  --
------------------------------------------------------------------------------

package = newpackage()
package.name = "lua"
package.kind = "exe"
package.language = "c++"

package.includepaths = { "include" }
package.defines = { "LUA_OPNAMES", "WIN32", "_LIB", "NDEBUG", "LUAPLUS_LIB" }

package.files = { "src/lua/lua.c" }
package.links = { "lualib" }

