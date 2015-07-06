## Orbit Pages

Orbit Pages is a PHP-like programming environment built on top of Orbit. Orbit pages
are HTML pages (but using the extension .op in a typical Orbit installation) that
get dynamically converted into Orbit apps. They are launched by the op.cgi, op.fcgi
and ophandler.lua launchers, for CGI, FastCGI, and Xavante, respectively. A standard
Kepler installation includes support for Orbit pages by default.

An Orbit page is also a template that uses the [Cosmo](http://cosmo.luaforge.net)
template language. The environment of this template is a sandbox that wraps the
global environment and is recreated on each request. The main variable in this
environment is the `web` variable, which is an Orbit request/response object.
Other variables of note:

**mapper** - an instance of the default Orbit ORM

**model(*name*, *dao*)** - same as mapper:new(*name*, *dao*), except that if *name*
is a tabel then calls mapper:new(*name[1]*, *name[2]*), so you can use this in the
template as `$model{ name, dao }`

**app** - the application's global environment, to be used as a session cache (for DB
connections, for example) for persistent launchers

**finish(*res*)** - suspends the execution of the current page, and sends *res* as a
response **instead** of the page's contents

**redirect(*target*)** - same as web:redirect(*target*) followed by finish(). If *target*
is a table does web:redirect(*target[1]*), so you can use this in the template as
`$redirect{ target }`

**include(*page*, [*env*])** - evaluates the Orbit page in the file *page* (relative to the
current page's path), optionally using the extra variables in *env* in the template's
environment. Can also be used in the template as `$include{ page, env }`

**forward(*page*, [*env*])** - aborts the execution of the current page and evaluates
and sends the page in file *page* instead; otherwise same as **include**

There also a few more variables that should be used only in the template:

**$lua{ *code* }** - runs *code* in the same environment as the page, so
*code* can change the template's variables and even define new ones

**$if{ *condition* }[[ *then-part* ]],[[ *else-part* ]]** - if *condition* is true then
is replaced by the template evaluation of *then-part*, otherwise *else-part*. *else-part*
is optional, defaulting to blank

**$fill{ ... }[[ *template* ]]** - replaced by the evaluation of *template* using the environment
passed to fill (*template* does **not** inherit the variables of the page)

Below is a very simple Orbit page that shows most of the concepts above (including Cosmo
concepts, see the Cosmo documentation for that):

        #!/usr/bin/env op.cgi
        <html>
        <body>
        <p>Hello Orbit!</p>
        <p>I am in $web|real_path, and the script is
        $web|script_name.</p>
        $lua{[[
          if not web.input.msg then
            web.input.msg = "nothing"
          end
        ]]}
        <p>You passed: $web|input|msg.</p>
        $include{ "bar.op" }
        </body>
        </html>

The `bar.op` page it includes is this:

        #!/usr/bin/env op.cgi
        <p>This is bar, and you passed $web|input|msg!</p>

The [Kepler](http://www.keplerproject.org) distribution has a more complete example that has database
access, POST, and even some simple AJAX.
