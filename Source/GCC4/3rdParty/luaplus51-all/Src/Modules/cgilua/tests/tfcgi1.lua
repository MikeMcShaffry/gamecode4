-- $Id: tfcgi1.lua,v 1.1 2004/03/25 19:31:05 tomas Exp $
lfcgi.stdout:write"Content-type: text/html\n\n"

for i,v in pairs{"QUERY_STRING", } do
	lfcgi.stdout:write (string.format ("%s = %s", v, os.getenv(v) or '&nbsp;'))
end
lfcgi.stdout:write "<br>\n"

local post_data = lfcgi.stdin:read"*a"
lfcgi.stdout:write (string.format ("post_data = {%s}", post_data))
