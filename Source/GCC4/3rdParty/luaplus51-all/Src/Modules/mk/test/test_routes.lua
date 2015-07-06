
local R = require "mk.routes"

do
   local r = R('/foo')
   local t = r:match("/foo")
   assert(t)
   assert(r:build() == "/foo")
end

do
   local r = R('/foo')
   local t = r:match("/bar")
   assert(not t)
   assert(r:build() == "/foo")
end

do
   local r = R('/foo')
   local t = r:match("/foobar")
   assert(not t)
end

do
   local r = R("/foo/bar/:baz")
   local t = r:match("/foo/bar/boo")
   assert(t.baz == "boo")
   assert(r:build{ baz = "boo"} == "/foo/bar/boo")
   assert(not pcall(r.build, r))
end

do
   local r = R("/foo/bar/:baz")
   local t = r:match("/bar/boo")
   assert(not t)
end

do
   local r = R("/foo/bar/:baz")
   local t = r:match("/foo/bar/boo/bloo")
   assert(not t)
end

do
   local r = R("/say/:msg/to/:to")
   local t = r:match("/say/hello/to/world")
   assert(t.msg == "hello")
   assert(t.to == "world")
   assert(r:build{ msg = "hello", to = "world" } == "/say/hello/to/world")
   assert(r:build{ msg = "hello", to = 5 } == "/say/hello/to/5")
end

do
   local r = R('/say/*/to/*')
   local t = r:match('/say/hello/to/world')
   assert(#t.splat == 2)
   assert(t.splat[1] == "hello")
   assert(t.splat[2] == "world")
   assert(r:build{ splat = { "hello", "world" } } == "/say/hello/to/world")
end

do
   local r = R('/download/*.*')
   local t = r:match('/download/path/to/file.xml')
   assert(#t.splat == 2)
   assert(t.splat[1] == "path/to/file")
   assert(t.splat[2] == "xml")
   assert(r:build{ splat = { "path/to/file", "xml" } } == "/download/path/to/file.xml")
end

do
   local r = R('/*/foo/*/*')
   local t = r:match('/bar/foo/bling/baz/boom')
   assert(#t.splat == 3)
   assert(t.splat[1] == "bar")
   assert(t.splat[2] == "bling")
   assert(t.splat[3] == "baz/boom")
   assert(r:build{ splat = { "bar", "bling", "baz/boom" } } == "/bar/foo/bling/baz/boom")
end

do
   local r = R('/*/foo/*')
   local t = r:match('/bar/foo/bling/baz/boom')
   assert(#t.splat == 2)
   assert(t.splat[1] == "bar")
   assert(t.splat[2] == "bling/baz/boom")
   assert(r:build{ splat = { "bar", "bling/baz/boom" } } == "/bar/foo/bling/baz/boom")
end

do
   local r = R('/*/foo/*')
   local t = r:match('/bar/foo/')
   assert(#t.splat == 1)
   assert(t.splat[1] == "bar")
   assert(r:build{ splat = { "bar", "bling/baz/boom" } } == "/bar/foo/bling/baz/boom")
end

do
   local r = R('/*/foo/*')
   local t = r:match('/bar/foo')
   assert(#t.splat == 1)
   assert(t.splat[1] == "bar")
   assert(r:build{ splat = { "bar", "bling/baz/boom" } } == "/bar/foo/bling/baz/boom")
end

do
   local r = R('/:foo/*')
   local t = r:match('/foo/bar/baz')
   assert(#t.splat == 1)
   assert(t.foo == "foo")
   assert(t.splat[1] == "bar/baz")
   assert(r:build{ foo = "foo", splat = { "bar/baz" } } == "/foo/bar/baz")
end

do
   local r = R('/:foo/:bar')
   local t = r:match('/user@example.com/name')
   assert(t.foo == "user@example.com")
   assert(t.bar == "name")
   assert(r:build{ foo = "user@example.com", bar = "name" } == "/user@example.com/name")
end

do
   local r = R('/:foo.:bar')
   local t = r:match('/user@example.com')
   assert(t.foo == "user@example")
   assert(t.bar == "com")
   assert(r:build{ foo = "user@example", bar = "com" } == "/user@example.com")
end

do
   local r = R('/*')
   local t = r:match("/foo/bar/baz")
   assert(t.splat[1] == "foo/bar/baz")
   assert(r:build{ splat = { "foo/bar/baz" } } == "/foo/bar/baz")
end

do
   local r = R('/*')
   local t = r:match("/")
   assert(not t.splat[1])
   assert(r:build{ splat = { "foo/bar/baz" } } == "/foo/bar/baz")
end

do
   local r = R('/*')
   local t = r:match("/")
   assert(not t.splat[1])
   assert(r:build{} == "/")
end

do
   local r = R('/*')
   local t = r:match("")
   assert(not t.splat[1])
   assert(r:build{ splat = { "foo/bar/baz" } } == "/foo/bar/baz")
end

do
   local r = R('/?:foo?/?:bar?')
   local t = r:match('/hello/world')
   assert(t.foo == 'hello')
   assert(t.bar == 'world')
   assert(r:build{ foo = "hello", bar = "world" } == "/hello/world")
end

do
   local r = R('/?:foo?/?:bar?')
   local t = r:match('/hello')
   assert(t.foo == 'hello')
   assert(not t.bar)
   assert(r:build{ foo = "hello" } == "/hello")
end

do
   local r = R('/?:foo?/?:bar?')
   local t = r:match('/')
   assert(not t.foo)
   assert(not t.bar)
   assert(r:build() == "/")
end

do
   local r = R('/:foo/*')
   local t = r:match('/hello%20world/how%20are%20you')
   assert(t.foo == "hello world")
   assert(t.splat[1] == "how are you")
   assert(r:build{ foo = "hello world", splat = { "how are you" } } == '/hello+world/how+are+you')
end 
