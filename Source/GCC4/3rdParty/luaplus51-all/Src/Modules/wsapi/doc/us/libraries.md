## Overview

WSAPI includes a set of helper libraries to make writing applications and web frameworks easier.
To use these libraries just `require` them in your application or framework.

## Request

**wsapi.request.new(*wsapi\_env*, [*options*])** - creates a new request object wrapping *wsapi\_env*; *options* is an (optional) table of extra options for the request; the *delay\_post* option tells wsapi.request to not process POST data on creation, the *overwrite*
option tells WSAPI to overwrite input parameters with the same name instead of collecting them in a list, and the *mk\_app* option
passes the [mk](http://github.com/keplerproject/mk) application that created the request object, used in some of the methods

**req:parse\_post()** - processed the POST data in case the processing
was delayed by passing *delay\_post = true* on creation of the request

**req:qd\_encode(*tab*)** - encodes *tab* as a query string

**req:route\_link(*route*, *tab*, ...)** - creates a link to mk route *route*, encoding *tab* as the query string and passing
extra arguments to the link builder of the route

**req:link\_&lt;route&gt;(*tab*, ...)** - same as **req:route\_link(*route*, *tab*, ...)**

**req:link(*uri*, *tab*)** - makes an internal application link to the specified resource *uri*, with *tab* encoded as
a query string. For example, if the app is addressed by /foo/bar.lua then **req:link("/baz", { id = 2 })** returns
"/foo/bar.lua/baz?id=2"

**req:static\_link(*uri*, *tab*)** - as **req:link**, but builds a link external to the application. In the previous
example it would return "/foo/baz?id=2"

**req:absolute\_link(*url*, *tab*)** - just encodes *tab* as a query string and appends it to *url*. Use it to make non-decorated
links

**req:empty(*s*)** - returns `true` if *s* is `nil` or a blank string, `false` otherwise

**req:empty\_param(*name*)** - same as **req:empty(req.params[*name*])**

**req.GET** - table with GET parameters of request

**req.POST** - table with POST parameters of request

**req.method** - request method (usually "GET" or "POST")

**req.path\_info** - PATH\_INFO metavariable

**req.script\_name** - SCRIPT\_NAME metavariable

**req.doc\_root** - DOCUMENT\_ROOT metavariable

**req.env** - the original WSAPI environment

**req.app\_path** - the path of the WSAPI application currently running

**req.query\_string** - unparsed query string

**req.params** - union of **req.GET** and **req.POST**, built on demand

**req.cookies[*name*]** - gets value of a cookie from browser

## Response

**wsapi.response.new([*status*, *headers*])** - creates a new response
object, optionally setting an initial status code and header table. If a Content-Type was
not passed in the initial header table then sets it as "text/html". The default status code
is 200

**res.status** - status code to be returned to server

**res.headers** - table with headers to be returned to server

**res:content\_type(*mime*)** - sets the Content-Type header to *mime*

**res:write(...)** - adds the arguments to the body, flattening an argument if it is a table

**res:set\_cookie(*name*, *value*)** - sets the value of a cookie, *value* can be either a string
or a table with fields *value*, *expires* (expiration date), *domain*, *path*, and *secure*. All fields
except *value* are optional

**res:delete\_cookie(*name*, *path*)** - tells the browser to erase a cookie, with an optional *path*

**res:redirect(*url*)** - sets status and headers for a redirect response to *url*, and returns a WSAPI response
that does the redirect

**res:forward(*uri*)** - sets the PATH\_INFO metavariable to *uri* (if not nil) and returns a mk FORWARD response to
tell [mk](http://github.com/keplerproject/mk) to keep trying to find a request handler

**res:finish()** - finishes response, returning status, headers and an iterator for the body

## Util

**wsapi.util.url\_encode(*s*)** - encodes *s* according to RFC2396

**wsapi.util.url\_decode(*s*)** - decodes *s* according to RFC2396

**wsapi.util.sanitize(*text*)** - sanitizes all HTML tags in *text*, replacing &lt; and &gt; with the
corresponding entity codes

**wsapi.util.not\_empty(*s*)** - returns `true` if *s* is not `nil` or the empty string

**wsapi.util.getopt(*arg*, *options*)** - POSIX style command line argument parser, *arg* contains the command line arguments
in a standard table, *options* is a string with the letters that expect string values. Returns a table with the options that
have been passed and their values

**wsapi.util.make\_env\_get(*qs*)** - makes a mock WSAPI environment with GET method and *qs* as the query string

**wsapi.util.make\_env\_post(*pd*, *type*, *qs*)** - makes a mock WSAPI environment with POST method and *pd* as the postdata,
*type* as the encoding (x-www-form-urlenconded default), and *qs* as the query string

**wsapi.util.make\_rewindable(*wsapi\_env*)** - wraps *wsapi\_env* in a new
environment that lets you process the POST data more than once. This new
environment's input object has a *rewind* method that you can call to allow you to read
the POST data again.

## Mock

**make\_handler(*wsapi\_app*)** - Creates a mock handler for testing the WSAPI application you pass in.

The resulting handler will be a table with three fields:

**app** - The app itself.

**get** - A function to perform GET requests.

**post** - A function to perform POST requests.

The `get` and `post` functions both accept the following arguments:

**path** (required) - The path to request. Do not include the query string, this is specified in `params`.

**params** (optional) - A table of query or form data parameters.

**headers** (optional) - Any request headers you wish to specify.