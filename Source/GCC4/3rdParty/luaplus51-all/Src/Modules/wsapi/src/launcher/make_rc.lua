
io.write("#define IDS_WSAPI 1\r\n")
io.write("STRINGTABLE\r\nBEGIN\r\n")
io.write("IDS_WSAPI \"")

for line in io.lines((...)) do
  if not line:match("^#!") then
    line = line:gsub("\\", "\\\\"):gsub('"', '""'):gsub("[\r\n]+", "")
    io.write(line .. "\\n\\\r\n")
  end
end

io.write("\"\r\nEND\r\n")
