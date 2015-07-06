# Introduction

A recent (Jan 2006) discussion on the mailing list has prompted me to attempt to design an extended API which extends the Lua API by adding functions to the os and io namespaces.

This is not a proposal to modify the Lua core, but a design proposal for an API which extends the Lua core. This API is meant to provide a more complete programming environment for stand-alone Lua programs on today's popular operating systems (Windows, MacOSX and POSIX platforms).






# Credits

The primary contents of this document come from <http://lua-users.org/wiki/ExtensionProposal>.





# ex API

Note that all these functions return the standard (nil,"error message") on failure and that, unless otherwise specified, they return (true) on success.






## Environment

### os.getenv(varname)

Returns the value of the process environment variable `varname` or `nil` if the variable is not defined.

<pre>
    print(os.getenv("PATH"))
</pre>



### os.setenv(varname, value)

Sets the environment variable `varname` to be `value`.  If `value` is `nil`, the environment variable is removed.  Note: The change occurs only within the currently running process.

<pre>
	print(os.getenv("foo"))          -- Prints: nil
    print(os.setenv("foo", "bar"))
	print(os.getenv("foo"))          -- Prints: bar
    print(os.setenv("foo", nil))
	print(os.getenv("foo"))          -- Prints: nil
</pre>




### env = os.environ()

Returns a copy of the environment as a Lua table of key-value pairs.

<pre>
    env = os.environ()
    print(env.HOME)
</pre>



-------------------------



## File system (mostly borrowing from LuaFilesystem?)

### os.access(path [ , mode ])

Determines if the file specified by `path` is read-only or not.  Returns

`mode` is optional or can be one of the following:

* If not specified, the file is checked for existence only.
* `w` - Checks whether the file is write-only.
* `r` - Checks whether the file is read-only.
* `rw` - Checks whether the file is readable and writeable.

Returns true if the file matches the mode, false otherwise.

<pre>
    print(os.access('myfile.txt'))      -- Prints true if myfile.txt exists.
    print(os.access('myfile.txt', 'w')  -- Prints true if myfile.txt is write-only.
    print(os.access('myfile.txt', 'r')  -- Prints true if myfile.txt is read-only.
    print(os.access('myfile.txt', 'rw') -- Prints true if myfile.txt is readable and writeable.
</pre>




### os.chdir(pathname)

Change the working directory to `pathname`.

<pre>
    os.chdir(os.getenv("HOME"))
</pre>




### os.chmod(pathname, mode)

Changes the file `mode` of `pathname`.

`mode` may be one of the following bitfields:

* `400` - Allow read by owner.
* `200` - Allow write by owner.
* `100` - Allow execution by owner.
* `040` - Allow read by group members.
* `020` - Allow write by group members.
* `010` - Allow execution by group members.
* `004` - Allow read by others.
* `002` - Allow write by others.
* `001` - Allow execution by others.

`mode` may also be symbolic:

* `w` - Writable.
* `r` - Readable.
* `rw` - Both read and writable.

<pre>
    os.chmod('script', 777)

    os.chmod('writablefile', 'w')
</pre>




### cwd = os.getcwd()

Retrieves the current working directory.

<pre>
    cwd = os.getcwd()
</pre>




### os.mkdir(pathname)

Create the directory described by `pathname`.  `pathname` is in the form of `dirA/dirB/dirC/` or `dirA/dirB/dirC/filename`.  All directories up to the final slash (or backslash) are created.  The filename component is ignored.

<pre>
    dirName = 'dirA/dirB/dirC'
    os.mkdir(dirName)                    -- Creates dirA/dirB/
    dirName = 'dirA/dirB/dirC/'
    os.mkdir(dirName)                    -- Creates dirA/dirB/dirC/
    filename = 'dirA/dirB/dirC/filename'
    os.mkdir(filename)                   -- Creates dirA/dirB/dirC/
</pre>




### os.remove(pathname)

Removes the file or directory at `pathname`.  If `pathname` is a directory, all child directories and files within those directories are destroyed.

<pre>
    os.remove('dirA/dirB/')              -- Remove dirB/ recursively
    os.remove('dirA/file.txt')           -- Remove file.txt from dirA/
</pre>




### os.copyfile(srcfilename, destfilename)

Copies the file named `srcfilename` to `destfilename` preserving file attributes and timestamp.

<pre>
    ex.copyfile('filea.txt', 'fileb.txt')    -- Copy filea.txt to fileb.txt
</pre>





### ex.copydirectory(srcdirectory, destdirectory)

Copies the directory named `srcdirectory` into `destdirectory` preserving file attributes and timestamps.  This function differs from `os.mirrordirectory` in that the `srcdirectory` files and directories are overlayed onto `destdirectory`.  `os.mirrordirectory` removes extra files and directories.

<pre>
    ex.copydirectory('/dira', '/dirb')
</pre>





### ex.mirrordirectory(srcdirectory, destdirectory)

Mirrors the directory named `srcdirectory` to `destdirectory` preserving file attributes and timestamps.  `os.mirrordirectory` removes extra files and directories.

<pre>
    ex.mirrordirectory('/dira', '/dirb')
</pre>





### os.movefile(srcfilename, destfilename)

Moves the file named `srcfilename` to `destfilename`.

<pre>
    os.movefile('filea.txt', 'fileb.txt')    -- Move filea.txt to fileb.txt
</pre>







--------------

## I/O (locking and pipes)

### file:lock(mode, offset, length)
### io.lock(file, mode, offset, length)

Lock or unlock a file or a portion of a file; 'mode' is either "r" or "w" or "u"; 'offset' and 'length' are optional.  A mode of "r" requests a read lock, "w" requests a write lock, and "u" releases the lock. Note that the locks may be either advisory or mandatory.





### file:unlock(offset, length)
### io.unlock(file, offset, length)

Equivalent to file:lock("u", offset, length) or io.lock(file, "u", offset, length).

Note that both file:lock() and file:unlock() extend the metatable for Lua file objects.




### rd, wr = io.pipe()

Create a pipe; 'rd' and 'wr' are Lua file objects.




----------------


## Process control

### os.sleep(seconds)
### os.sleep(interval, unit)

Suspends program execution for interval/unit seconds; 'unit' defaults to 1 and either argument can be floating point. The particular sub-second precision is implementation-defined.

<pre>
    os.sleep(3.8) -- sleep for 3.8 seconds
    local microseconds = 1e6
    os.sleep(3800000, microseconds) -- sleep for 3800000 µs
    local ticks = 100
    os.sleep(380, ticks) -- sleep for 380 ticks
</pre>





### proc = os.spawn(filename)
### proc = os.spawn{filename, [args-opts]}
### proc = os.spawn{command=filename, [args-opts]}

Creates a child process.

`filename` names a program. If the (implementation-defined) pathname is not absolute, the program is found through an implementation-defined search method (the PATH environment variable on most systems).

If specified, [args-opts] is one or more of the following keys:

* `[1]..[n]=` - the command line arguments
* `args=` - an array of command line arguments
* `env=` - a table of environment variables
* `stdin=`, `stdout=`, `stderr=` - io.file objects for standard input, output, and error respectively
* `shell=` - Set to `true` if this is a shell application.  Set to `false` if this is a GUI application.  Defaults to `true`.
* `show=` - Set to `true` to show the application.  Set to `false` to hide it.  Defaults to `true`.

It is an error if both integer keys and an 'args' key are specified.

An implementation may provide special behavior if a zeroth argument (options.args[0] or options[0]) is provided.

The returned 'proc' userdatum has the method `wait()`.

<pre>
    -- run the echo command
    proc = os.spawn"/bin/echo"
    proc = os.spawn{"/bin/echo", "hello", "world"}
    proc = os.spawn{command="/bin/echo", "hello", "world"}

    -- run the id command
    vars = { LANG="fr_FR" }
    proc = os.spawn{"/bin/id", "-un", env=vars}
    proc = os.spawn{command="/bin/id", "-un", env=vars)

    -- Useless use of cat
    local rd, wr = assert(io.pipe())
    local proc = assert(os.spawn("/bin/cat", {stdin=rd}))
    rd:close()
    wr:write("Hello world\n")
    wr:close()
    proc:wait()

    -- Run a program with a modified environment
    local env = os.environ()
    env.LUA_PATH = "/usr/share/lib/lua/?.lua"
    env.LUA_CPATH = "/usr/share/lib/lua/?.so"
    local proc = assert(os.spawn("lua", {args = {"-e", 'print"Hello world\n"'}, env=env }))
    proc:wait()

    -- popen2()
    function popen2(...)
        local in_rd, in_wr = io.pipe()
        local out_rd, out_wr = io.pipe()
        local proc, err = os.spawn{stdin = in_rd, stdout = out_wr, ...}
        in_rd:close(); out_wr:close()
        if not proc then
            in_wr:close(); out_rd:close()
            return proc, err
        end
        return proc, out_rd, in_wr
    end

    -- usage:
    local p, i, o = assert(popen2("wc", "-w"))
    o:write("Hello world"); o:close()
    print(i:read"*l"); i:close()
    p:wait()
</pre>



### exitcode = proc:wait()

Wait for child process termination; 'exitcode' is the code returned by the child process.




### ex.collectlines{filename, [args-opts]}
### ex.collectlines{command=filename, [args-opts]}

Returns a table of all output from the new process's stdout.

See ex.spawn() for additional information.





### ex.lines{filename, [args-opts]}
### ex.lines{command=filename, [args-opts]}

Returns an iterator that reads the new process's stdout line by line.

See ex.spawn() for additional information.





### proc, input = ex.popen{filename, [args-opts]}
### proc, input = ex.popen{command=filename, [args-opts]}

Creates a child process with the process's stdout redirected to be read from `input`.

See ex.spawn() for additional information.





### proc, input, output = ex.popen2{filename, [args-opts]}
### proc, input, output = ex.popen2{command=filename, [args-opts]}

Creates a child process with the process's stdout redirected to be read from `input` and the process's stdin redirected to be written to through `output`.

See ex.spawn() for additional information.





### args = os.parsecommandline(commandline)

Parses a single string command line into a table of arguments suitable for passing to `os.spawn`.

<pre>
    args = os.parsecommandline('dir "c:\\Program Files\\"')
</pre>


-------------------------



## Path Manipulation (os.path)

### os.path.add_extension(path, extension)

Adds `extension` to the end of `path` even if one already exists.

Returns the new path.

<pre>
    assert(os.path.add_extension('', 'ext') == '.ext')
    assert(os.path.add_extension(' ', ' ext') == ' . ext')
    assert(os.path.add_extension('', '.ext') == '.ext')
    assert(os.path.add_extension('hello', 'ext') == 'hello.ext')
    assert(os.path.add_extension('hello', '.ext') == 'hello.ext')
    assert(os.path.add_extension('hello.txt', 'ext') == 'hello.txt.ext')
    assert(os.path.add_extension('hello.txt', '.ext') == 'hello.txt.ext')
</pre>





### os.path.add_slash(path)

Adds a slash to the end of `path` if it doesn't already exist.

Returns the new path.

<pre>
    assert(os.path.add_slash('') == '/')
    assert(os.path.add_slash(' ') == ' /')
    assert(os.path.add_slash('hello') == 'hello/')
    assert(os.path.add_slash(' hello') == ' hello/')
    assert(os.path.add_slash(' hello ') == ' hello /')
    assert(os.path.add_slash('hello/') == 'hello/')
</pre>





### os.path.append(leftPath, rightPath)

Appends `leftPath` and `rightPath` together, adding a slash between the two path components.  If `rightPath` is an absolute path, it is not appended to `leftPath` and is returned directly.

Returns the new path.

<pre>
    assert(os.path.append('', 'filename.txt') == 'filename.txt')
    assert(os.path.append('', 'dir', 'filename.txt') == 'dir/filename.txt')
    assert(os.path.append('', 'dirA', '', 'dirB', 'filename.txt') == 'dirA/dirB/filename.txt')
    assert(os.path.append('..', 'filename.txt') == '../filename.txt')
    assert(os.path.append('root', 'filename.txt') == 'root/filename.txt')
    assert(os.path.append('root', 'dir', 'filename.txt') == 'root/dir/filename.txt')
    assert(os.path.append('root', 'dirA', '', 'dirB', 'filename.txt') == 'root/dirA/dirB/filename.txt')
    assert(os.path.append('root', 'dirA', '', 'dirB', '..', 'filename.txt') == 'root/dirA/dirB/../filename.txt')
    assert(os.path.append('root', 'dirA', '', '/dirB', '..', 'filename.txt') == '/dirB/../filename.txt')
</pre>





### os.path.combine(leftPath, rightPath)

Combines `leftPath` and `rightPath`, adding a slash between the two path components and simplifying the path by collapsing `.` and `..` directories.  If `rightPath` is an absolute path, it is not appended to `leftPath` and is returned directly.

Returns the new path.

<pre>
    assert(os.path.combine('', 'filename.txt') == 'filename.txt')
    assert(os.path.combine('', 'dir', 'filename.txt') == 'dir/filename.txt')
    assert(os.path.combine('', 'dirA', '', 'dirB', 'filename.txt') == 'dirA/dirB/filename.txt')
    assert(os.path.combine('..', 'filename.txt') == '../filename.txt')
    assert(os.path.combine('root', 'filename.txt') == 'root/filename.txt')
    assert(os.path.combine('root', 'dir', 'filename.txt') == 'root/dir/filename.txt')
    assert(os.path.combine('root', 'dirA', '', 'dirB', 'filename.txt') == 'root/dirA/dirB/filename.txt')
    assert(os.path.combine('root', 'dirA', '', 'dirB', '..', 'filename.txt') == 'root/dirA/filename.txt')
</pre>





### os.path.escape(path)

Properly escapes and quotes `path`, if needed.

Returns the new path.

<pre>
    assert(os.path.escape('') == '')
    assert(os.path.escape(' ') == '" "')
    assert(os.path.escape('filename.txt') == 'filename.txt')
    assert(os.path.escape('file name.txt') == '"file name.txt"')
</pre>





### os.path.exists(path)

Returns *true* if `path` exists, *false* otherwise.

<pre>
    assert(os.path.exists('filename.txt') == true)
</pre>





### os.path.find_extension(path)

For the given `path`, return the index of the extension.  Returns `nil` if the `path` has no extension.

<pre>
    assert(os.path.find_extension('') == nil)
    assert(os.path.find_extension('filename') == nil)
    assert(os.path.find_extension('.lua') == 1)
    assert(os.path.find_extension('pathtests.lua') == 10)
    assert(os.path.find_extension('pathtests') == nil)
</pre>






### os.path.find_filename(path)

For the given `path`, return the index of the filename.

<pre>
    assert(os.path.find_filename('pathtests.lua') == 1)
    assert(os.path.find_filename('/pathtests') == 2)
    assert(os.path.find_filename('c:/pathtests') == 4)
    assert(os.path.find_filename('c:pathtests') == 3)
</pre>





### os.path.get_extension(path)

For the given `path`, return the extension.

<pre>
    assert(os.path.get_extension('') == '')
    assert(os.path.get_extension('filename') == '')
    assert(os.path.get_extension('filename.ext') == '.ext')
    assert(os.path.get_extension('filename.txt.ext') == '.ext')
</pre>





### os.path.get_filename(path)

For the given `path`, return just the filename without the directory.

<pre>
    assert(os.path.get_filename(''), '')
    assert(os.path.get_filename('filename'), 'filename')
    assert(os.path.get_filename('filename.ext'), 'filename.ext')
    assert(os.path.get_filename('c:/directory/filename.ext'), 'filename.ext')
    assert(os.path.get_filename('c:/directory/'), '')
</pre>





### os.path.is_directory(path)

Returns *true* if `path` is a directory, *false* otherwise.

<pre>
    assert(os.path.is_directory('') == false)
    assert(os.path.is_directory(scriptPath .. 'pathtests.lua') == false)
    assert(os.path.is_directory('.') == true)
    assert(os.path.is_directory('..') == true)
    assert(os.path.is_directory(scriptPath .. '../tests') == true)
    assert(os.path.is_directory(scriptPath .. '../tests/') == true)
</pre>





### os.path.is_file(path)

Returns *true* if `path` is a file, *false* otherwise.

<pre>
    assert(os.path.is_file('') == nil)
    assert(os.path.is_file(scriptPath .. 'pathtests.lua') == true)
    assert(os.path.is_file('.') == false)
    assert(os.path.is_file('..') == false)
    assert(os.path.is_file(scriptPath .. '../tests') == false)
    assert(os.path.is_file(scriptPath .. '../tests/') == nil)
</pre>





### os.path.is_relative(path)

Returns *true* if `path` is relative, *false* otherwise.

<pre>
    assert(os.path.is_relative('') == true)
    assert(os.path.is_relative('filename.ext') == true)
    assert(os.path.is_relative('/filename.ext') == false)
    assert(os.path.is_relative('c:/filename.ext') == false)
</pre>






### os.path.is_root(path)

Returns *true* if `path` is a root path, *false* otherwise.

<pre>
    assert(os.path.is_root('') == false)
    assert(os.path.is_root('filename.ext') == false)
    assert(os.path.is_root('/filename.ext') == true)
    assert(os.path.is_root('c:/filename.ext') == true)
</pre>






### os.path.is_unc(path)

Returns *true* if `path` is a UNC path, *false* otherwise.

<pre>
    assert(os.path.is_unc('') == false)
    assert(os.path.is_unc('filename.ext') == false)
    assert(os.path.is_unc('/filename.ext') == false)
    assert(os.path.is_unc('c:/filename.ext') == false)
    assert(os.path.is_unc('\\\\share') == true)
    assert(os.path.is_unc('//share') == true)
</pre>






### os.path.is_writable(path)

Returns *true* if `path` is writable, *false* if read-only.

<pre>
    local writable = os.path.is_writable('filename.dat')
</pre>





### os.path.make_absolute(path)

Converts `path` into an absolute path, simplifying the path as necessary.

Returns the new path.

<pre>
    local cwd = os.getcwd():gsub('\\', '/')
    assert(os.path.make_absolute('') == cwd)
    assert(os.path.make_absolute('.') == cwd .. '/')
    assert(os.path.make_absolute('..') == cwd:match('(.+)/') .. '/')
    assert(os.path.make_absolute('abc') == cwd .. '/abc')
</pre>






### os.path.make_backslash(path)

Convert any forward slashes in `path` to backslashes.

Returns the new path.

<pre>
    assert(os.path.make_backslash('') == '')
    assert(os.path.make_backslash(' ') == ' ')
    assert(os.path.make_backslash('\\\\abc') == '\\\\abc')
    assert(os.path.make_backslash('//abc') == '\\\\abc')
    assert(os.path.make_backslash('c:/abc/def/') == 'c:\\abc\\def\\')
</pre>






### os.path.make_slash(path)

Convert any backslashes in `path` to slashes.

Returns the new path.

<pre>
    assert(os.path.make_slash('') == '')
    assert(os.path.make_slash(' ') == ' ')
    assert(os.path.make_slash('\\\\abc') == '//abc')
    assert(os.path.make_slash('//abc') == '//abc')
    assert(os.path.make_slash('c:\\abc\\def\\') == 'c:/abc/def/')
</pre>






### os.path.make_writable(path)

Make the file *path* writable that is read-only.

Returns *true* if the process succeeded.

<pre>
    os.path.make_writable('filename.dat')
</pre>






### os.path.match(path, wildcard)

Returns *true* if the `wildcard` matches `path`, *false* otherwise.

<pre>
    assert(os.path.match('', '') == true)
    assert(os.path.match('', '*') == true)
    assert(os.path.match('', '*.*') == false)
    assert(os.path.match('', 'a*') == false)
    assert(os.path.match('abcdefg.txt', 'a*') == true)
    assert(os.path.match('abcdefg.txt', 'a*b*c?e*') == true)
    assert(os.path.match('abcdefg.txt', 'a*b*c?f*') == false)
    assert(os.path.match('abcdefg.txt', '*.') == false)
    assert(os.path.match('abcdefg.txt', '*.t') == false)
    assert(os.path.match('abcdefg.txt', '*.t*') == true)
    assert(os.path.match('abcdefg.txt', '*.t') == false)
    assert(os.path.match('abcdefg.txt', '*.*t') == true)
    assert(os.path.match('abcdefg.txt', '*.*x') == false)
    assert(os.path.match('abcdefg.txt', '*.txt') == true)
</pre>






### os.path.remove_directory(path)

Removes the directory component from `path`.

Returns the new path.

<pre>
    assert(os.path.remove_directory('') == '')
    assert(os.path.remove_directory(' \t') == ' \t')
    assert(os.path.remove_directory('abc') == 'abc')
    assert(os.path.remove_directory('/abc.') == 'abc.')
    assert(os.path.remove_directory('/dir/abc.') == 'abc.')
    assert(os.path.remove_directory('c:/abc.') == 'abc.')
    assert(os.path.remove_directory('c:/dir/abc') == 'abc')
</pre>






### os.path.remove_extension(path)

Removes the extension component from `path`.

Returns the new path.

<pre>
    assert(os.path.remove_extension('') == '')
    assert(os.path.remove_extension(' \t') == ' \t')
    assert(os.path.remove_extension('abc') == 'abc')
    assert(os.path.remove_extension('abc.') == 'abc')
    assert(os.path.remove_extension('abc.txt') == 'abc')
    assert(os.path.remove_extension('abc.txt.dat') == 'abc.txt')
</pre>






### os.path.remove_filename(path)

Removes the filename component from `path`.

Returns the new path.

<pre>
    assert(os.path.remove_filename('') == '')
    assert(os.path.remove_filename(' \t') == '')
    assert(os.path.remove_filename('abc') == '')
    assert(os.path.remove_filename('/abc.') == '/')
    assert(os.path.remove_filename('/dir/abc.') == '/dir/')
    assert(os.path.remove_filename('c:/abc.') == 'c:/')
    assert(os.path.remove_filename('c:/dir/abc') == 'c:/dir/')
</pre>






### os.path.remove_slash(path)

Removes the trailing slash from `path`.

Returns the new path.

<pre>
    assert(os.path.remove_slash('abc') == 'abc')
    assert(os.path.remove_slash('abc/') == 'abc')
</pre>






### os.path.replace_extension(path, extension)

Replaces the extension of `path` with the new one specified in `extension`.

Returns the new path.

<pre>
    assert(os.path.replace_extension('', 'ext') == '.ext')
    assert(os.path.replace_extension('', '.ext') == '.ext')
    assert(os.path.replace_extension('hello', 'ext') == 'hello.ext')
    assert(os.path.replace_extension('hello', '.ext') == 'hello.ext')
    assert(os.path.replace_extension('hello.txt', 'ext') == 'hello.ext')
    assert(os.path.replace_extension('hello.txt', '.ext') == 'hello.ext')
    assert(os.path.replace_extension('hello.txt.dat', 'ext') == 'hello.txt.ext')
    assert(os.path.replace_extension('hello.txt.dat', '.ext') == 'hello.txt.ext')
</pre>






### os.path.simplify(path)

Simplifies `path` by collapsing `.` and `..` directories and removing multiple directory slashes.

Returns the new path.

<pre>
    assert(os.path.simplify('') == '')
    assert(os.path.simplify('abc') == 'abc')
    assert(os.path.simplify('.abc') == '.abc')
    assert(os.path.simplify('./abc') == 'abc')
    assert(os.path.simplify('..abc') == '..abc')
    assert(os.path.simplify('../abc') == '../abc')
    assert(os.path.simplify('abc/////def') == 'abc/def')
    assert(os.path.simplify('abc/././././def') == 'abc/def')
    assert(os.path.simplify('c:/somedir/.././././def') == 'c:/def')
    assert(os.path.simplify('abc/.././././def') == 'def')
    assert(os.path.simplify('abc/ABC/../../../../def') == 'def')
    assert(os.path.simplify('c:\\abc\\ABC\\../..\\../..\\def') == 'c:/def')
    assert(os.path.simplify('\\\\uncserver\\pathA\\file.txt') == '\\\\uncserver/pathA/file.txt')
</pre>






### os.path.split(path)

Splits `path` into directory and filename components.  Returns both the directory and filename.

<pre>
    function compare_split(path, expectedDirname, expectedFilename)
    	local actualDirname, actualFilename = os.path.split(path)
    	return actualDirname == expectedDirname  and  actualFilename == expectedFilename
    end

    assert(compare_split('', '', '') == true)
    assert(compare_split('abc.txt', '', 'abc.txt') == true)
    assert(compare_split('/', '/', '') == true)
    assert(compare_split('/abc', '/', 'abc') == true)
    assert(compare_split('/abc/', '/abc/', '') == true)
    assert(compare_split('c:/', 'c:/', '') == true)
    assert(compare_split('c:/abc', 'c:/', 'abc') == true)
    assert(compare_split('c:/abc/', 'c:/abc/', '') == true)
</pre>






### os.path.trim(path)

Trims whitespace from the beginning and end of `path`.

Returns the new path.

<pre>
    assert(os.path.trim('abc.txt') == 'abc.txt')
    assert(os.path.trim(' abc.txt') == 'abc.txt')
    assert(os.path.trim('abc.txt ') == 'abc.txt')
    assert(os.path.trim('  \t  abc.txt \t \t \t ') == 'abc.txt')
</pre>






### os.path.unescape(path)

Unescapes `path`.

Returns the new path.

<pre>
    assert(os.path.unescape('') == '')
    assert(os.path.unescape('"') == '')
    assert(os.path.unescape('""') == '')
    assert(os.path.unescape('" "') == ' ')
    assert(os.path.unescape('"file with spaces') == 'file with spaces')
    assert(os.path.unescape('"file with spaces"') == 'file with spaces')
</pre>






