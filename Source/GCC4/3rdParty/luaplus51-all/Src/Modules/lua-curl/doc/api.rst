==================
Lua-cURL 
==================

:Author: Jürgen Hötzel
:Contact: http://www.hoetzel.info/
:Date: $Date: 2007/09/21 16:52:59 $
:Copyright: This document has been placed in the public domain.

.. contents::

Introduction
------------------

This project is **not** a fork of LuaCURL_, which is a direct mapping of parts of the libcurl-easy_ interface.

.. _libcurl-easy: http://curl.haxx.se/libcurl/c/libcurl-easy.html
.. _LuaCURL: http://luaforge.net/projects/luacurl/

The intent of Lua-cURL is to adapt the

* Easy Interface
* Multi Interface
* Shared Interface 

of libcurl_ to the functionality of Lua (for example by using iterators instead of callbacks when possible).

.. _libcurl: http://curl.haxx.se/libcurl/c/

Installation
------------------

The Autotools chain is used to configure, build and install. Just invoke::

    ./configure && make install 


If your Lua installation doesn't contain pkg-config support, you have to specify compiler/linker flags and the target directory::

    LUA_CFLAGS="-I/usr/local/include" LUA_LIBS="-L/usr/local/lib -lm -llua" ./configure  --with-cmoddir=/usr/local/lib/lua/5.1

Easy interface
------------------

**cURL.easy_init()**
  returns a new easy handle.

**cURL.version_info()**
  returns a table containing version info and features/protocols sub table

**easy:escape(string)**
  return URL encoded string

**easy:unescape(string)**
  return URL decoded string

**easy:setopt\*(value)**
  libcurl properties an options are mapped to individual functions: 
   * **easy:setopt_url(string)**
   * **easy:setopt_verbose(number)**
   * **easy:setopt_proxytype(string)**
   * **easy:setopt_share(share)**  (See: share_)
   * ...

**easy:perform(table)**
  Perform the transfer as described in the options, using an optional callback table.The callback table indices are named after the equivalent cURL callbacks:
   *  **writefunction = function(str)**
   *  **readfunction = function()**
   *  **headerfunction = function(str)**

**easy:post(table)**
  Prepare a multipart/formdata post. The table indices are named after the form fields and should map to string values::

    {field1 = value1,
     field2 = value1}
  
  or more generic descriptions in tables::

    {field1 = {file="/tmp/test.txt",
               type="text/plain"},
    {field2 = {file="dummy.html",
               data="<html><bold>bold</bold></html>,
               type="text/html"}}


Example 1: Fetch the example.com homepage
.........................................

..  include:: ../examples/file.lua  
    :literal:


Example 2: "On-The-Fly" file upload
...................................

..  include:: ../examples/ftpupload.lua
    :literal:

Example 3: "Posting" data
.........................

.. include:: ../examples/post.lua
   :literal:


Multi interface
--------------------

**cURL.multi_init()** 
  returns a new multi handle

**multi:add_handle(easy)**
  add an easy handle to a multi session

**multi:perform()**
  returns an iterator function that, each time it is called, returns the next data, type and corresponding easy handle:
  
    **data**:
       data returned by the cURL library
    **type**
       type of data returned ("header" or "data")      
    **easy**
       corresponding easy handle of the data returned

 
Example 1: "On-The-Fly" XML parsing
.........................................

.. include:: ../examples/rss.lua
   :literal:

Share interface
--------------------
.. _share:

**cURL.share_init()** 
  returns a new share handle

**share:setopt_share(string)**
  specifies the type of data that should be shared ("DNS" or "COOKIE")

Since Lua is single-threaded, there is no mapping for the lock options.

Example 1: Share Cookie date across easy handles
................................................

.. include:: ../examples/share.lua
   :literal:


Appendix
----------------------------------------

Using SSL
.........................

The cert bundle distributed with cURL may be out of date and cannot validate many certificates. You can supply a different 
PEM cert bundle by using **easy:setopt_cainfo(string)**.
I wrote a shell script (download_) to convert the cacert keystore distributed with the Java Runtime Environment to PEM.

.. _download: http://www.hoetzel.info/Hacking/Bash%20scripting/keystore2pem.sh



