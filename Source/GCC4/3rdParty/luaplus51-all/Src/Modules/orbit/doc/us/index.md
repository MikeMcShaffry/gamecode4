## Overview

Orbit is an MVC web framework for Lua. The design is inspired by lightweight Ruby
frameworks such as [Camping](http://code.whytheluckystiff.net/camping/). It completely
abandons the CGILua model of "scripts" in favor of applications, where each Orbit
application can fit in a single file, but you can split it into multiple files if you want.
All Orbit applications follow the [WSAPI](http://keplerproject.github.com/wsapi) protocol, so they currently
work with Xavante, CGI and Fastcgi. It includes a launcher that makes it easy to launch
a Xavante instance for development.

## History

* Version 2.2.0 (31/Mar/2010)
    * Reparse response to resume the dispatcher
    * better parser for orbit.model conditions, fixes parsing bugs
    * `orbit` launcher has parameters to control logging and port
    * `op.cgi`/`op.fcgi` launchers have the same parameters as `wsapi.cgi`/`wsapi.fcgi`
    * Optional [Sinatra](http://www.sinatrarb.com/)-like route parser, using LPEG
    * Pluggable route parsers (route patterns can be strings or objects that answer to :match)

* Version 2.1.0 (29/Oct/2009)
    * better decoupling of orbit and orbit.model
    * support for anything with a match method as patterns
    * new options for orbit.model finders: distinct, fields
    * count option for orbit.model now limits number of rows in the SQL
    * logging of queries in orbit.model
    * overhaul of the "orbit" script: better options, --help, sets application paht
    * content_type method in the web object to set content type
    * support for PUT and DELETE (methods `dispatch_put` and `dispatch_delete`)
    * orbit.model.recycle(*conn_builder*, *timeout*) function, to make a connection that
      automatically reopens after a certain time
    * more samples in the samples folder
    * added a "higher-order" $if to Orbit Pages

* Version 2.0.2 (10/Mar/2009)
    * url-decodes path captures (suggested by Ignacio Burgueño on a Jul 24 email to the Kepler list)
    * added tutorial and new examples
    * fixed escape.string
    * web:delete_cookie receives a path parameter in order to correctly remove the cookie. Bug report and patch by Ignacio Burgueño
    * stripping UTF-8 BOM from templates read from disk
    * removing SoLazer files in order to make the Orbit package smaller
    * added alternate name for integer (int)
    * better error reporting for missing escape and convert functions
    * removed toboolean
    * fixed bugs 13451 and 25418: setting status 500 on application errors not throwing an error if file not exists when invalidating cache

* Version 2.0.1 (10/Jun/2008): bug-fix release, fixed bug in Orbit pages' redirect function (thanks for
Ignacio Burgueno for finding the bug)

* Version 2.0 (06/Jun/2008): Complete rewrite of Orbit

* Version 1.0: Initial release, obsolete

## Download and Installation

The easiest way to download and install Orbit is via [LuaRocks](http://luarocks.org). You 
can install Orbit with a simple `luarocks install orbit`. Go to the path where LuaRocks
put Orbit to see the sample apps and this documentation. LuaRocks will automatically fetch
and install any dependencies you don't already have.

Do not be alarmed by the size of the Orbit package (~1MB), Orbit itself is a approximately 1%
of that; most of the package are samples, and a single one is responsible for 50% of the total
size because it embeds the Sproutcore JavaScript framework.

## Hello World

Below is a very simple Orbit application:

<pre>
    #!/usr/bin/env wsapi.cgi

    require"orbit"

    -- Orbit applications are usually modules,
    -- orbit.new does the necessary initialization

    module("hello", package.seeall, orbit.new)

    -- These are the controllers, each receives a web object
    -- that is the request/response, plus any extra captures from the
    -- dispatch pattern. The controller sets any extra headers and/or
    -- the status if it's not 200, then return the response. It's
    -- good form to delegate the generation of the response to a view
    -- function

    function index(web)
      return render_index()
    end

    function say(web, name)
      return render_say(web, name)
    end

    -- Builds the application's dispatch table, you can
    -- pass multiple patterns, and any captures get passed to
    -- the controller

    hello:dispatch_get(index, "/", "/index")
    hello:dispatch_get(say, "/say/(%a+)")

    -- These are the view functions referenced by the controllers.
    -- orbit.htmlify does through the functions in the table passed
    -- as the first argument and tries to match their name against
    -- the provided patterns (with an implicit ^ and $ surrounding
    -- the pattern. Each function that matches gets an environment
    -- where HTML functions are created on demand. They either take
    -- nil (empty tags), a string (text between opening and
    -- closing tags), or a table with attributes and a list
    -- of strings that will be the text. The indexing the
    -- functions adds a class attribute to the tag. Functions
    -- are cached.
    --

    -- This is a convenience function for the common parts of a page

    function render_layout(inner_html)
       return html{
         head{ title"Hello" },
         body{ inner_html }
       }
    end

    function render_hello()
       return p.hello"Hello World!"
    end
    
    function render_index()
       return render_layout(render_hello())
    end

    function render_say(web, name)
       return render_layout(render_hello() .. 
         p.hello((web.input.greeting or "Hello ") .. name .. "!"))
    end

    orbit.htmlify(hello, "render_.+")

    return _M
</pre>

Save this to `hello.lua`, install `wsapi-xavante` with LuaRocks, and now you can run this application in two ways:
the first is to run `wsapi` in the same directory that you saved the file and point your browser to `http://localhost:8080/hello.lua`,
or run `orbit hello.lua` in the same directory that you saved the file and point your browser to `http://localhost:8080/`. Now try
appending `index`, `say/foo`, and `say/foo?message=bar` to the URL.

The example uses Orbit's built-in html generation, but you are free to use any method of generating HTML. 
One of Orbit's sample applications uses the [Cosmo](http://cosmo.luaforge.net) template library, for instance.

## OR Mapping

Orbit also includes a basic OR mapper that currently only works with 
[LuaSQL's](http://github.com/keplerproject/luasql) SQLite3 and MySQL drivers. The mapper provides
dynamic find methods, a la Rails' ActiveRecord (find\_by\_field1\_and\_field2{val1, val2}),
as well as templates for conditions (find_by("field1 = ? or field1 = ?", { val1, val2 })). 
The sample applications use this mapper.

A nice side-effect of the Orbit application model is that we get an "application console" 
for free. For example, with the blog example we can add a new post like this:

<pre>
    $ lua -l luarocks.require -i blog.lua
    > p = blog.posts:new()
    > p.title = "New Post"
    > p.body = "This is a new blog post. Include *Markdown* markup freely."
    > p.published_at = os.time()
    > p:save()
</pre>

You can also update or delete any of the model items right from your console, just fetch 
them from the database, change what you want and call `save()` 
(or `delete()` if you want to remove it).

## Credits

Orbit was designed and developed by Fabio Mascarenhas and André Carregal,
and is maintained by Fabio Mascarenhas.

## Contact Us

For more information please [contact us](mailto:info-NO-SPAM-THANKS@keplerproject.org).
Comments are welcome!

You can also reach us and other developers and users on the Kepler Project 
[mailing list](http://luaforge.net/mail/?group_id=104). 
