@goto =run

print("Hello!")
for key, value in ipairs(arg) do print(key, value) end

--[[
:=run
@LuaPlus.exe %~f0 %*
::]]