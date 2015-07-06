require "ex"

function AddAutoExp(autoexpFileName)
	local ignore = false

	local file = io.open(autoexpFileName .. ".new", "wt")

	if os.path.exists(autoexpFileName) then
		for line in io.lines(autoexpFileName) do
			if line == '; -- LuaPlus and Lua begin --' then
				ignore = true
			elseif line == '; -- LuaPlus and Lua end --' then
				ignore = false
			elseif line == '; This section lets you define your own errors for the HRESULT display.' then
				for newLine in io.lines('AutoExpAdditions.txt') do
					file:write(newLine .. "\n")
				end

				file:write(line .. "\n")
			else
				if not ignore then
					file:write(line .. "\n")
				end
			end
		end

		file:close()
	end

	os.copyfile(autoexpFileName .. ".new", autoexpFileName)
	os.remove(autoexpFileName .. ".new")
end


function AddVSAutoExp(environmentVariable)
	local VS = os.getenv(environmentVariable)
	if not VS then return end

	VS = VS .. '..\\..\\'

	AddAutoExp(VS .. "\\Common7\\Packages\\Debugger\\autoexp.dat")
end


AddVSAutoExp("VS80COMNTOOLS")
AddVSAutoExp("VS90COMNTOOLS")
AddVSAutoExp("VS100COMNTOOLS")

local XEDK = os.getenv("XEDK")
if XEDK then
	AddAutoExp(XEDK .. "\\bin\\win32\\autoexp.dat")
end
