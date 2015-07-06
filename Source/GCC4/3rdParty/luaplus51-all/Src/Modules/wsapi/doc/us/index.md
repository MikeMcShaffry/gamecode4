## Overview

*WSAPI* is an API that abstracts the web server from Lua web applications. By coding
against WSAPI your application can run on any of the supported servers and
interfaces (currently CGI, FastCGI and Xavante, on Windows and UNIX-based systems).

WSAPI provides a set of helper libraries that help with request processing
and output buffering. You can also write applications that act as filters that
provide some kind of service to other applications, such as authentication,
file uploads, request isolation, or multiplexing.

WSAPI's main influence is Ruby's [Rack](http://rack.rubyforge.org/) framework, but it was
also influenced by Python's [WSGI](http://wsgi.org/wsgi) (PEP 333). It's not a direct
clone of either of them, though, and tries to follow standard Lua idioms.

WSAPI is free software and uses the same license as Lua 5.1.

## Status

Current version is 1.3.4. It was developed for Lua 5.1.

## Download

You can get WSAPI using [LuaRocks](http://luarocks.org):

<pre class="example">
luarocks install wsapi-xavante
</pre>

### Unix Installer Script

You can also get an installer script that installs Lua+LuaRocks+WSAPI 
[here](http://github.com/downloads/keplerproject/wsapi/wsapi-install-1.3.4.tar.gz). See
the [manual](manual.html) for installation instructions.

### Customizing the installer

There is a section of wsapi-install-1.3.4 with the parameters that 
control the installer: 

<pre class="example">
# Installer parameters 

LUA_VERSION=5.1.4 
PACKAGE=WSAPI 
PACKAGE_OPT=wsapi 
PACKAGE_ROCK=wsapi-xavante 
INSTALLER_VERSION=0.6 
PACKAGE_VERSION=1.3.4
LUAROCKS_REPO=http://luarocks.org/repositories/rocks
LUAROCKS_URL=http://www.luarocks.org/releases/luarocks-2.0.2.tar.gz
LUAROCKS_VERSION=2.0.2 
</pre>

To install something else change PACKAGE to the full name of the 
package, PACKAGE\_OPT to the name of the --with-foo option that lets 
the user override the version (or skip installation of the package), 
PACKAGE\_ROCK to the name of the rock, and PACKAGE\_VERSION to the 
version. Also change LUAROCKS\_REPO if you want to use another 
repository (the installer uses --from, so will pull packages from 
other repositories if the one you specified does not have them). 

If there is a LuaRocks update then change LUAROCKS\_URL and 
LUAROCKS\_VERSION. Changing Lua version is much more involved, so I 
won't go into that. 

Now to make the tarball, put the installer script in an empty folder and run: 

<pre class = "example">
bash ./your-install-script --prefix=/tmp/anything --bootstrap 
</pre>

After it finishes you will have lua-5.1.4.tar.gz, 
luarocks-2.0.2.tar.gz, and a rocks folder with .src.rocks for all the 
rocks that the installer installs. 

## Latest Sources and Bug Tracker

WSAPI sources and bug tracker are available at its [Github](http://github.com/keplerproject/wsapi/) page.

## History

**WSAPI 1.4** [TBR]

* Refactoring of request, response, and util modules, to work with [mk](http://github.com/keplerproject/mk)
* New methods for `wsapi.request`: qs_encode (encodes a table as a query string), route_link (makes a link to
  a mk route, link (makes an internal app link), absolute_link (makes an absolute link), static_link (makes
  an external link), empty (checks if a string is just blanks or nil), empty_param (checks if a request param
  is empty), and renamed parse_post_data method to parse_post
* New methods for `wsapi.response`: forward (sets path_info and returns an mk "keep parsing" code), content_type
  (sets Content-Type header), redirect (sets Location header and returns redirect WSAPI response), changed
  write to take multiple parameters and flatten nested tables
* New functions in `wsapi.util`: make_env_get (makes a mock WSAPI environment for a GET request from a query
  string), make_env_post (makes a mock WSAPI environment for a POST request from a postdata string, a postdata
  type, default x-www-form-urlencoded, and a query string)

**WSAPI 1.3.4** [23/Mar/2010]

* Bugfix release, fix bug introduced by previous fix :-)

**WSAPI 1.3.3** [22/Mar/2010]

* Bugfix release, fix memory leak with reload option for persistent loaders

**WSAPI 1.3.2** [19/Mar/2010]

* Bugfix release, do not send Transfer-Encoding header unless it is
  raw HTTP response

**WSAPI 1.3.1** [19/Mar/2010]

* Bugfix release, fix the unknown symbol in lfcgi.getenv()

**WSAPI 1.3** [18/Mar/2010]

* Fixed segmentation fault when non-string is provided to lfcgi.getenv() (thanks to mkottman@github)
* Added CGILua bootstrap to wsapi.sapi, so CGILua can run without a kepler_init module present
* Added an `extra_vars` paremeter to wsapi.xavante.makeHandler and wsapi.xavante.makeGenericHandler, to
  let you pass extra variables in the WSAPI environment
* Added `overwrite` option to wsapi.request that tells the parameter parser to overwrite repeated parameters
  instead of collecting them in a list    
* Added a parameter `isolated` to the persistent generic loaders that controls whether you isolate
  each script in a Lua state or not
* Added parameters to the persistent generic loaders that let the user control the life cycle of Lua
  states: `period` sets how long WSAPI should wait between collecting stale states, and `ttl` sets the
  period after which a state becomes stale
* Fixed bug in wsapi.ringer that didn't let you use wsapi.input:read inside the response iterator
* Parameter `vars` for the WSAPI generic loaders that which variables WSAPI should check to get the physical
  path of the script, and in which order. Defaults tro trying SCRIPT\_FILENAME first and PATH\_TRANSLATED second

**WSAPI 1.2** [27/Oct/2009]

* Adds time-based collection of Lua states to FCGI and Xavante launchers
* Adds "wsapi" laucher script, to start a Xavante WSAPI server
* Fixed "undefined media type" error
* Added is_empty utility function to check if a string is nil or ''
* Fixed bug with empty bodies in wsapi.xavante, and added full http status codes to responses 
* Changing order of evaluating PATH\_TRANSLATED and SCRIPT\_FILENAME, to make non-wrapped launchers work in OSX Apache
* Reload support for load\_isolated\_launcher

**WSAPI 1.1** [04/Feb/2009]

* Adds *options* table to **wsapi.request.new**, *delay_post* option delays
POST processing until **req:parse_post_data()** is called
* Moves call to **lfs.setmode** from wsapi.common to wsapi.cgi
* Adds **wsapi.util.make\_rewindable(*wsapi\_env*)** method - wraps *wsapi\_env* in a new
environment that lets you process the POST data more than once.
* Correctly handles PATH\_TRANSLATED and SCRIPT\_FILENAME in case the web server gets creative
* Statically links the FastCGI version on Windows

[**WSAPI 1.0**](http://wsapi.luaforge.net/1.0/) [18/May/2008]

* First public version.
* Includes CGI, FastCGI and Xavante WSAPI connectors.

## Credits

WSAPI was designed and developed by Fabio Mascarenhas and
Andr&eacute; Carregal, and is maintained by Fabio Mascarenhas.

## Contact Us

For more information please [contact us](mailto:info-NO-SPAM-THANKS@keplerproject.org).
Comments are welcome!

You can also reach us and other developers and users on the Kepler Project 
[mailing list](http://luaforge.net/mail/?group_id=104). 

