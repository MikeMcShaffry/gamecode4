local path = os.getenv("LUA_SOPATH")
if path then
	func = loadlib(path.."/lua-python.so", "luaopen_python")
	if func then
		func()
		return
	end
end
local modmask = "/usr/lib/python%d.%d/site-packages/lua-python.so"
local loaded = false
for i = 10, 2, -1 do
	for j = 10, 2, -1 do
		func = loadlib(string.format(modmask, i, j), "luaopen_python")
		if func then
			loaded = true
			func()
			break
		end
	end
end
if not loaded then
	error("unable to find python module")
end
