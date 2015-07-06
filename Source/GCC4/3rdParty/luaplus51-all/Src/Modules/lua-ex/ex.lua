module(..., package.seeall)

require 'ex.core'

-- ex.parsecommandline
function ex.parsecommandline(commandline)
	require 'lpeg'
	local field = lpeg.C('\"' * (lpeg.P(1) - '\"')^0 * '\"' + (1 - lpeg.P' ')^0)
	return lpeg.Ct(field * (lpeg.P(' ')^1 * field)^0 * -1):match(commandline)
end

-- ex.popen
function ex.popen(args, binary)
	local out_rd, out_wr = io.pipe(not binary)
	args.stdout = out_wr
	if args.stderr_to_stdout then
		args.stderr = out_wr
	end
	local proc, err = os.spawn(args)
	out_wr:close()
	if not proc then
		out_rd:close()
		return proc, err
	end
	return proc, out_rd
end

-- ex.lines
function ex.lines(args, binary)
	local proc, input = popen(args, not binary)
	return function()
		local line = input:read("*l")
		if line then return line end
		input:close()
		args.exitcode = proc:wait()
	end
end

-- ex.rawlines
function ex.rawlines(args, binary)
	local proc, input = popen(args, not binary)
	return function()
		local line = input:read(100)
		if line then return line end
		input:close()
		args.exitcode = proc:wait()
	end
end

-- ex.popen2()
function ex.popen2(args, binary)
	local in_rd, in_wr = io.pipe(not binary)
	local out_rd, out_wr = io.pipe(not binary)
	args.stdin = in_rd
	args.stdout = out_wr
	if args.stderr_to_stdout then
		args.stderr = out_wr
	end
	local proc, err = os.spawn(args)
	in_rd:close(); out_wr:close()
	if not proc then
		in_wr:close(); out_rd:close()
		return proc, err
	end
	return proc, out_rd, in_wr
end

-- ex.collectlines
function ex.collectlines(args)
	local lines = {}
	for line in ex.lines(args) do
		lines[#lines + 1] = line
	end
	args.lines = lines
	return lines
end


local function copy_directory_helper(srcPath, destPath, callback, deleteExtra)
	-- Create the destination directory.
	os.mkdir(destPath)

	-- Grab the destination directory contents.
	local destDirs = {}
	local destFiles = {}
	for destHandle in filefind.match(destPath .. "*.*") do
		local fileName = destHandle.filename
		if destHandle.is_directory then
			if fileName ~= '.'  and  fileName ~= '..' then
				destDirs[fileName:lower()] = true
			end
		else
			destFiles[fileName:lower()] = destHandle.write_time
		end
	end

	-- Scan the source directory.
	local srcDirs = {}
	local srcFiles = {}
	for srcHandle in filefind.match(srcPath .. "*.*") do
		local fileName = srcHandle.filename
		if srcHandle.is_directory then
			if fileName ~= '.'  and  fileName ~= '..' then
				srcDirs[#srcDirs + 1] = fileName
				destDirs[fileName:lower()] = nil
			end
		else
			local lowerFileName = fileName:lower()
			if srcHandle.write_time ~= destFiles[lowerFileName] then
				srcFiles[#srcFiles + 1] = fileName
			end
			destFiles[lowerFileName] = nil
		end
	end

	-- Delete any leftover destination files.
	if deleteExtra then
		for fileName in pairs(destFiles) do
			local destFullPath = destPath .. fileName
			if callback then callback('del', destFullPath) end
			os.remove(destFullPath)
		end
	end

	-- Copy any changed files.
	for _, fileName in ipairs(srcFiles) do
		local srcFileName = srcPath .. fileName
		local destFileName = destPath .. fileName
		if callback then callback('copy', srcFileName, destFileName) end
		os.chmod(destFileName, 'w')				-- Make sure we can overwrite the file
		os.copyfile(srcFileName, destFileName)
	end

	-- Delete any leftover destination directories.
	if deleteExtra then
		for dirName in pairs(destDirs) do
			local destFullPath = destPath .. dirName .. '/'
			if callback then callback('del', destFullPath) end
			os.remove(destFullPath)
		end
	end

	-- Recurse through the directories.
	for _, dirName in ipairs(srcDirs) do
		copy_directory_helper(srcPath .. dirName .. '/', destPath .. dirName .. '/', callback, deleteExtra)
	end
end


function ex.copydirectory(srcPath, destPath, callback)
	require 'filefind'

	srcPath = os.path.add_slash(os.path.make_slash(srcPath))
	destPath = os.path.add_slash(os.path.make_slash(destPath))

	copy_directory_helper(srcPath, destPath, callback, false)
end


function ex.mirrordirectory(srcPath, destPath, callback)
	require 'filefind'

	srcPath = os.path.add_slash(os.path.make_slash(srcPath))
	destPath = os.path.add_slash(os.path.make_slash(destPath))

	copy_directory_helper(srcPath, destPath, callback, true)
end


function ex.removeemptydirectories(path)
	require 'filefind'
	require 'ex'

	local dirs = {}
	local remove = true

	for handle in filefind.match(path .. "*.*") do
		if handle.is_directory then
			local fileName = handle.filename

			if fileName ~= '.'  and  fileName ~= '..' then
				dirs[#dirs + 1] = fileName
			end
		else
			remove = false
		end
	end

	for _, dirName in ipairs(dirs) do
		if not ex.removeemptydirectories(path .. dirName .. '\\') then
			remove = false
		end
	end

	if remove then
		os.remove(path)
	end

	return remove
end
