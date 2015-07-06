import "glob"

function RemoveDir(name)
	local table = glob.match("**" .. name .. "/")
	for k, v in table do
		path = v:sub(1, v:len() - 1)
		os.execute("rm -rf " .. path)
	end
end

RemoveDir("Debug")
RemoveDir("Release")
