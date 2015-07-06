-- $Id: tcgi1.lua,v 1.1 2004/03/25 19:31:05 tomas Exp $

io.stdout:write"Content-type: text/html\n\n"

for i,v in pairs{"QUERY_STRING", } do
	io.stdout:write (string.format ("%s = %s", v, os.getenv(v) or '&nbsp;'))
end
io.stdout:write "<br>\n"

local post_data = io.stdin:read"*a"
io.stdout:write (string.format ("post_data = {%s}", post_data))
