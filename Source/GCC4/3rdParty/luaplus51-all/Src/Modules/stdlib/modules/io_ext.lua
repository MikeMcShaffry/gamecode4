-- I/O

module ("io", package.seeall)

require "base"


-- @func readLines: Read a file into a list of lines and close it
--   @param [h]: file handle or name [io.input ()]
-- @returns
--   @param l: list of lines
function readLines (h)
  if h == nil then
    h = input ()
  elseif _G.type (h) == "string" then
    h = io.open (h)
  end
  local l = {}
  for line in h:lines () do
    table.insert (l, line)
  end
  h:close ()
  return l
end

-- @func writeLine: Write values adding a newline after each
--   @param [h]: file handle [io.output ()]
--   @param ...: values to write (as for write)
function writeLine (h, ...)
  if io.type (h) ~= "file" then
    table.insert (arg, 1, h)
    h = io.output ()
  end
  for _, v in ipairs (arg) do
    h:write (v, "\n")
  end
end

-- @func splitdir: split a directory path into components
-- Empty components are retained: the root directory becomes {"", ""}.
-- The same as Perl's File::Spec::splitdir
--   @param path: path
-- @returns
--   @param: path1, ..., pathn: path components
function splitdir (path)
  return string.split ("/", path)
end

-- @func catdir: concatenate directories into a path
-- The same as Perl's File::Spec::catdir
--   @param: path1, ..., pathn: path components
-- @returns
--   @param path: path
function catdir (...)
  local path = table.concat ({...}, "/")
  -- Suppress trailing / on non-root path
  return (string.gsub (path, "(.)/$", "%1"))
end

-- @func shell: Perform a shell command and return its output
--   @param c: command
-- @returns
--   @param o: output, or nil if error
function shell (c)
  local h = io.popen (c)
  local o
  if h then
    o = h:read ("*a")
    h:close ()
  end
  return o
end

-- @func processFiles: Process files specified on the command-line
-- If no files given, process io.stdin; in list of files, "-" means
-- io.stdin
--   @param f: function to process files with
--     @param name: the name of the file being read
--     @param i: the number of the argument
function processFiles (f)
  if #arg == 0 then
    table.insert (arg, "-")
  end
  for i, v in ipairs (arg) do
    if v == "-" then
      io.input (io.stdin)
    else
      io.input (v)
    end
    prog.file = v
    f (v, i)
  end
end
