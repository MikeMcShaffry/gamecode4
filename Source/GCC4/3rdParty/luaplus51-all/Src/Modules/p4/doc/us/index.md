## Introduction

P4Lua, a Lua interface to the Perforce API, enables you to write Lua code that interacts with a Perforce server. P4Lua enables your Lua scripts to:

* Get Perforce data and forms in Lua tables
* Edit Perforce forms by modifying Lua tables
* Provide optional exception-based error handling and optionally ignore warnings
* Issue multiple commands on a single connection (performs better than spawning single commands and parsing the results)





## System Requirements

P4Lua is supported on Windows.

To build P4Lua from source, your development machine must also have:

* Lua 5.1.4 development files
* The 2008.2 Perforce C/C++ API for your target platform
* The same C++ compiler used to build the Perforce C++ API on your target platform.  (If you get “unresolved symbol” errors when building or running P4Lua, you probably used the wrong compiler or the wrong Perforce API build. )

	


## Credits

The primary contents of this document come from Perforce 2008.2 APIs for Scripting.





## Programming with P4Lua

P4Lua provides an object-oriented interface to Perforce that is intended to be intuitive for Lua programmers. Data is loaded and returned in Lua tables. Each P4 object represents a connection to the Perforce Server.

When instantiated, the P4 instance is set up with the default environment settings just as the command line client p4, that is, using environment variables, the registry on Windows and, if defined, the `P4CONFIG` file. The settings can be checked and changed before the
connection to the server is established with the `connect()` method. After your script connects, it can send multiple commands to the Perforce Server with the same P4 instance. After the script is finished, it should disconnect from the Server by calling the
`disconnect()` method.

The following example illustrates the basic structure of a P4Lua script. The example establishes a connection, issues a command, and tests for errors resulting from the command.

<pre>
    require 'p4'                              -- Import the module
    p4 = P4.P4()                              -- Create the P4 instance
    p4.port = "1666"
    p4.user = "fred"
    p4.client = "fred-ws"                     -- Set some environment variables
    ret, err = pcall( function()              -- Catch exceptions
    	p4:connect()                          -- Connect to the Perforce Server
    	info = p4:run("info")                 -- Run "p4 info" (returns a table)
    	for key, value in pairs(info[1]) do   -- and display all key-value pairs
            print(key .. '=' .. value)
	    end
        p4:run("edit", "file.txt")            -- Run "p4 edit file.txt"
    end)

    for _, e in ipairs(p4.errors) do          -- Display errors
        print(e)
    end

    p4:disconnect()                       -- Disconnect from the Server
</pre>





This example creates a client workspace from a template and syncs it:.

<pre>
    require 'p4'

    template = "my-client-template"
    client_root = "c:\\work\\my-root"

    p4 = P4.P4()

    pcall(function()
        p4:connect()
        -- Convert client spec into a Lua dictionary
        client = p4:fetch_client("-t", template)
        client._root = client_root
        p4:save_client(client)
        p4:run_sync()
    end)
</pre>




### Submitting a Changelist

This example creates a changelist, modifies it and then submits it:

<pre>
    require 'p4'
    p4 = P4.P4()
    p4:connect()
    change = p4:fetch_change()

    -- Files were opened elsewhere and we want to
    -- submit a subset that we already know about.
    myfiles = { '//depot/some/path/file1.c', '//depot/some/path/file1.h' }
    change._description = "My changelist\nSubmitted from P4Lua\n"
    change._files = myfiles -- This attribute takes a Lua list
    p4:run_submit(change)
</pre>




### Logging into Perforce using ticket-based authentication

On some servers, users might need to log in to Perforce before issuing commands. The
following example illustrates login using Perforce tickets.

<pre>
    require 'p4'
    p4 = P4.P4()
    p4.user = "Sven"
    p4.password = "my_password"
    p4:connect()
    p4:run_login()
    opened = p4:run_opened()
    [...]
</pre>



### Changing your password

You can use P4Lua to change your password, as shown in the following example:

<pre>
    require 'p4'
    p4 = P4.P4()
    p4.user = "Joshua"
    p4.password = "MyOldPassword"
    p4:connect()
    p4:run_password("MyOldPassword", "MyNewPassword")
    -- p4.password is automatically updated with the encoded password
</pre>


-------------------------
# P4Lua Classes

The P4 module consists of several public classes:

* P4
* DepotFile
* Revision
* Integration
* MergeData
* Spec

The following tables provide more details about each public class.




## P4

Perforce client class. Handles connection and interaction with the Perforce server. There is one instance of each connection.

The following table lists attributes of the class `P4` in P4Lua. The attributes are read- and writable unless indicated otherwise. The attributes can be strings, objects, or integers.

<table>
  <tr><th>Attribute</th><th>Description</th></tr>
  <tr><td><tt>api_level</tt></td><td>API compatibility level. (Lock server output to a specified server level.)</td></tr>
  <tr><td><tt>charset</tt></td><td>Charset for Unicode servers.</td></tr>
  <tr><td><tt>client</tt></td><td><tt>P4CLIENT</tt>, the name of the client workspace to use.</td></tr>
  <tr><td><tt>cwd</tt></td><td>Current working directory</td></tr>
  <tr><td><tt>errors</tt></td><td>An array containing the error messages received during execution of the last command</td></tr>
  <tr><td><tt>exception_level</tt></td><td>The exception level of the P4 instance. Values can be
    <ul>
	  <li>0 : no exceptions are raised</li>
      <li>1 : only errors are raised as exceptions</li>
      <li>2: warnings are also raised as exceptions</li>
	</ul>
    The default value is 2.
  </td></tr>
  <tr><td><tt>host</tt></td><td><tt>P4HOST</tt>, the name of the host used</td></tr>
  <tr><td><tt>input</tt></td><td>Input for the next command. Can be a string or a table.</td></tr>
  <tr><td><tt>keepalive</tt></td><td>Early abort running Perforce commands.</td></tr>
  <tr><td><tt>maxlocktime</tt></td><td>MaxLockTime used for all following commands</td></tr>
  <tr><td><tt>maxresults</tt></td><td>MaxResults used for all following commands</td></tr>
  <tr><td><tt>maxscanrows</tt></td><td>MaxScanRows used for all following commands</td></tr>
  <tr><td><tt>os</tt></td><td>Returns the OS (read-only)</td></tr>
  <tr><td><tt>output</tt></td><td>An array containing the output messages received during execution of the last command</td></tr>
  <tr><td><tt>p4config_file</tt></td><td>The location of the configuration file used (`P4CONFIG`). This attribute is read-only.</td></tr>
  <tr><td><tt>password</tt></td><td><tt>P4PASSWD</tt>, the password used.</td></tr>
  <tr><td><tt>port</tt></td><td><tt>P4PORT</tt>, the port used for the connection</td></tr>
  <tr><td><tt>prog</tt></td><td>The name of the script</td></tr>
  <tr><td><tt>server_level</tt></td><td>Returns the current Perforce server level</td></tr>
  <tr><td><tt>tagged</tt></td><td>To disable tagged output for the following commands, set the value to 0 or false. By default, tagged output is enabled.</td></tr>
  <tr><td><tt>ticket_file</tt></td><td><tt>P4TICKETS</tt>, the ticket file location used</td></tr>
  <tr><td><tt>user</tt></td><td><tt>P4USER</tt>, the user under which the connection is run</td></tr>
  <tr><td><tt>version</tt></td><td>The version of the script</td></tr>
  <tr><td><tt>warnings</tt></td><td>An array containing the warning messages received during execution of the last command</td></tr>
</table>



The following table lists all public methods of the class `P4`. Many methods are wrappers around `P4:run()`, which sends a command to the Perforce Server. Such methods are provided for your convenience.

<table>
  <tr><th>Method</th><th>Description</th></tr>
  <tr><td><tt>connect()</tt></td><td>Connects to the Perforce Server.</td></tr>
  <tr><td><tt>connected()</tt></td><td>Returns <tt>true</tt> if connected and the connection is alive, otherwise <tt>false</tt>.</td></tr>
  <tr><td><tt>delete_<em>spectype</em>()</tt></td><td>Deletes the spec <em>spectype</em>. Equivalent to the command <tt>P4:run("<em>spectype</em>", "-d")</tt>.</td></tr>
  <tr><td><tt>disconnect()</tt></td><td>Disconnects from the Perforce Server.</td></tr>
  <tr><td><tt>env()</tt></td><td>Get the value of a Perforce environment variable, taking into account <tt>P4CONFIG</tt> files and (on Windows) the registry.</td></tr>
  <tr><td><tt>identify()</tt></td><td>Returns a string identifying the P4Lua module.</td></tr>
  <tr><td><tt>fetch_<em>spectype</em>()</tt></td><td>Fetches the spec <em>spectype</em>. Equivalent to the command <tt>P4:run("spectype", "-o")</tt>.</td></tr>
  <tr><td><tt>format_<em>spectype</em>()</tt></td><td>Converts the spec <em>spectype</em> into a string.</td></tr>
  <tr><td><tt>parse_<em>spectype</em>()</tt></td><td>Parses a string representation of the spec <em>spectype</tt> and returns a table.</td></tr>
  <tr><td><tt>run()</tt></td><td>Runs a command on the server. Needs to be connected, or an exception is raised.</td></tr>
  <tr><td><tt>run_<em>command</em>()</tt></td><td>Runs the command <em>command</em>. Equivalent to <tt>P4:run("command")</tt>.</td></tr>
  <tr><td><tt>run_filelog()</tt></td><td>This command returns a list of <tt>DepotFile</tt> objects.  Specialization for the <tt>run()</tt> command.</td></tr>
  <tr><td><tt>run_login()</tt></td><td>Logs in using the specified password.</td></tr>
  <tr><td><tt>run_password()</tt></td><td>Convenience method: updates the password. Takes two arguments: <em>oldpassword</em>, <em>newpassword</em></td></tr>
  <tr><td><tt>run_resolve()</tt></td><td>Interface to <tt>p4 resolve</tt>.</td></tr>
  <tr><td><tt>run_submit()</tt></td><td>Convenience method for submitting changelists. When invoked with a change spec, it submits the spec. Equivalent to :

<pre>
    p4.input = myspec
    p4:run("submit", "-i")
</pre>

</td></tr>

  <tr><td><tt>save_<em>spectype</em>()</tt></td><td>Saves the spec <em>spectype</em>. Equivalent to the command: <code>P4:run("spectype", "-i").</code>
</td></tr>
</table>




## P4.DepotFile

Container class returned by `P4:run_filelog()`. Contains the name of the depot file and a 
list of `P4.Revision` objects.

<table>
  <tr><th>Attribute</th><th>Description</th></tr>
  <tr><td><tt>depotFile</tt></td><td>Name of the depot file</td></tr>
  <tr><td><tt>revisions</tt></td><td>Table of Revision objects</td></tr>
</table>
  




## P4.Revision

Container class containing one revision of a `DepotFile` object.

<table>
  <tr><th>Attribute</th><th>Description</th></tr>
  <tr><td><tt>action</tt></td><td>Action that created the revision</td></tr>
  <tr><td><tt>change</tt></td><td>Changelist number</td></tr>
  <tr><td><tt>client</tt></td><td>Client workspace used to create this revision</td></tr>
  <tr><td><tt>desc</tt></td><td>Short change list description</td></tr>
  <tr><td><tt>depotFile</tt></td><td>The name of the file in the depot</td></tr>
  <tr><td><tt>digest</tt></td><td>MD5 checksum of the revision</td></tr>
  <tr><td><tt>fileSize</tt></td><td>File size of this revision</td></tr>
  <tr><td><tt>integrations</tt></td><td>List of `P4.Integration` objects</td></tr>
  <tr><td><tt>rev</tt></td><td>Revision</td></tr>
  <tr><td><tt>time</tt></td><td>Timestamp (as datetime.datetime object)</td></tr>
  <tr><td><tt>type</tt></td><td>File type</td></tr>
  <tr><td><tt>user</tt></td><td>User that created this revision</td></tr>
</table>





## P4.Integration

Container class containing one integration for a Revision object

<table>
  <tr><th>Attribute</th><th>Description</th></tr>
  <tr><td><tt>how</tt></td><td>Integration method (merge/branch/copy/ignored)</td></tr>
  <tr><td><tt>file</tt></td><td>Integrated file</td></tr>
  <tr><td><tt>srev</tt></td><td>End revision</td></tr>
  <tr><td><tt>erev</tt></td><td>Start revision</td></tr>
</table>
  
  
  


## P4.Map

A class that allows users to create and work with Perforce mappings without requiring a 
connection to the Perforce Server. 

<table>
  <tr><th>Method</th><th>Description</th></tr>
  <tr><td><tt>P4.Map()</tt></td><td>Construct a new Map object (class method)</td></tr>
  <tr><td><tt>P4.Map_join()</tt></td><td>Joins two maps to create a third (class method)</td></tr>
  <tr><td><tt>clear()</tt></td><td>Empties a map</td></tr>
  <tr><td><tt>count()</tt></td><td>Construct a new Map object (class method)</td></tr>
  <tr><td><tt>is_empty()</tt></td><td>Tests whether or not a map object is empty</td></tr>
  <tr><td><tt>insert()</tt></td><td>Inserts an entry into the map</td></tr>
  <tr><td><tt>translate()</tt></td><td>Translate a string through a map</td></tr>
  <tr><td><tt>includes()</tt></td><td>Tests whether a path is mapped</td></tr>
  <tr><td><tt>reverse()</tt></td><td>Returns a new mapping with the left and right sides reversed</td></tr>
  <tr><td><tt>lhs()</tt></td><td>Returns the left side as an array</td></tr>
  <tr><td><tt>rhs()</tt></td><td>Returns the right side as an array</td></tr>
  <tr><td><tt>as_array()</tt></td><td>Returns the map as an array</td></tr>
</table>





## P4.Spec

Class allowing access to the fields in a Perforce specification form.

<table>
  <tr><th>Attribute</th><th>Description</th></tr>
  <tr><td><tt>spec.<em>_fieldname</em></tt></td><td>Value associated with the field named fieldname.</td></tr>
  <tr><td><tt>spec.permitted_fields</tt></td><td>Array containing the names of the fields that are valid for this spec object</td></tr>
</table>







----------------------
# Class P4
## Description

Main interface to the Lua client API.

This module provides an object-oriented interface to the Perforce SCM system. Data is 
returned in Lua tables and input can also be supplied in these formats.

Each `P4` object represents a connection to the Perforce Server, and multiple commands 
may be executed (serially) over a single connection (which of itself can result in 
substantially improved performance if executing lots of perforce commands).

1. Instantiate your `P4` object 
2. Specify your Perforce client environment: `client`, `host`, `password`, `port`, `user`
3. Set any options to control output or error handling: `exception_level`
4. Connect to the Perforce Server
5. Run your Perforce commands
6. Disconnect from the Perforce Server



## Instance Attributes

### p4.api_level -> int

Contains the API compatibility level desired. This is useful when writing scripts using 
Perforce commands that do not yet support tagged output. In these cases, upgrading to a 
later server that supports tagged output for the commands in question can break your 
script. Using this method allows you to lock your script to the output format of an older
Perforce release and facilitate seamless upgrades. Must be called before calling 
`P4:connect()`.

For the API integer levels that correspond to each Perforce release, see:
<http://kb.perforce.com/?article=512>.

<pre>
    require 'p4'
    p4 = P4.P4()
    p4.api_level = 57
    p4:connect()
    ...
    p4:disconnect()
</pre>




### p4.charset -> string

Contains the character set to use when connect to a Unicode enabled server. Do not use 
when working with non-Unicode-enabled servers. By default, the character set is the 
value of the `P4CHARSET` environment variable. If the character set is invalid, this method 
raises an exception.

<pre>
    require 'p4'
    p4 = P4.P4()
    p4.client = "www"
    p4.charset = "iso8859-1"
    p4:connect()
    p4:run_sync()
    p4:disconnect()
</pre>




### p4.client -> string

Contains the name of your client workspace. By default, this is the value of the `P4CLIENT` 
taken from any `P4CONFIG` file present, or from the environment according to the normal 
Perforce conventions.





### p4.cwd -> string

Contains the current working directly. Can be called prior to executing any Perforce 
command. Sometimes necessary if your script executes a chdir() as part of its 
processing.

<pre>
    require 'p4'
    p4 = P4.P4()
    p4.cwd = "/home/sven"
</pre>





### p4.errors -> list (read-only)

Returns an array containing the error messages received during execution of the last 
command.

<pre>
    require 'p4'

    p4 = P4.P4()
    pcall(function()
    	p4:connect()
    	p4.exception_level = 1 -- ignore "File(s) up-to-date"
    	files = p4:run_sync()
    end)

    for _, e in ipairs(p4.errors) do
    	print(e)
    end

    p4:disconnect()
</pre>





### p4.exception_level -> int

Configures the events which give rise to exceptions. The following three levels are 
supported:

* 0 disables all exception handling and makes the interface completely procedural; you 
are responsible for checking the `p4.errors` and `p4.warnings` arrays.
* 1 causes exceptions to be raised only when errors are encountered.
* 2 causes exceptions to be raised for both errors and warnings. This is the default.

For example

<pre>
    require 'p4'
    p4 = P4.P4()
    p4.exception_level = 1
    p4:connect() -- Exception on failure
    p4:run_sync() -- File(s) up-to-date is a warning - no exception raised
    p4:disconnect()
</pre>




### p4.host -> string

Contains the name of the current host. It defaults to the value of `P4HOST` taken from any 
`P4CONFIG` file present, or from the environment as per the usual Perforce convention. 
Must be called before connecting to the Perforce server.

<pre>
    require 'p4'
    p4 = P4.P4()
    p4.host = "workstation123.perforce.com"
    p4:connect()
    ...
    p4:disconnect()
</pre>





### p4.input -> string | table

Contains input for the next command.

Set this attribute prior to running a command that requires input from the user. When the 
command requests input, the specified data is supplied to the command. Typically, 
commands of the form `p4 cmd -i` are invoked using the `P4:save_spectype` methods, 
which retrieve the value from `p4.input` internally; there is no need to set `p4.input` when 
using the `P4:save_spectype` shortcuts.

You may pass a string, a hash table, or (for commands that take multiple inputs from the user) 
an array of strings or hash tables. If you pass an array, note that the first element of the array 
will be popped each time Perforce asks the user for input.

For example, the following code supplies a description for the default changelist and then 
submits it to the depot:

<pre>
    require 'p4'
    p4 = P4.P4()
    p4:connect()
    change = p4:run_change( "-o" )[1]
    change.Description = "Autosubmitted changelist"
    p4.input = change
    p4:run_submit( "-i" )
    p4:disconnect()
</pre>





### p4.keepalive -> function

In order to early abort a Perforce command run, a keep alive function must be set.  Returning
`true` from the keep alive function will allow the Perforce command to continue running.
Returning `false` will abort the command.  The keep alive function is called every 0.5 second.

This example will abort the sync command immediately.

<pre>
    require 'p4'
    p4 = P4.P4()
    p4:connect()
    p4.keepalive = function(self)
        -- abort immediately
        return false
    end
    files = p4:run_sync('...')
    p4.keepalive = nil  -- Remove the keep alive function for future commands
    p4:disconnect()
</pre>

This example uses two additional Lua modules, the multithreading Lanes and the GUI library
wxWidgets, to allow an early abort of the user's sync by pressing a button on a dialog.

<pre>
    require 'lanes'
    require 'wx'

    -- This is our communications pipeline with the thread.  It is inherited
    -- as an upvalue for the runp4 lane.
    local linda = lanes.linda()

    -- Create the lane, giving it full access to all the standard libraries.
    runp4 = lanes.gen('*', function()
        require 'p4'
        p4 = P4.P4()

        pcall(function()
            p4:connect()
            p4.keepalive = function()
                -- If the linda receives the finished message, then abort.
                if linda:receive(0, "finished") then return false end
                return true
            end
            files = p4:run{ 'sync', '...', OutputText = function(info) print(info.clientFile) end }
            p4.keepalive = nil
        end)

        for _, w in ipairs(p4.warnings) do
            print(w)
        end

        for _, e in ipairs(p4.errors) do
            print(e)
        end

        p4:disconnect()
    end)

    -- Start the thread.
    p4result = runp4()

    -- Pop up the dialog.
    wx.wxMessageBox("Press OK to cancel sync.")

    -- After the dialog OK button has been pressed, send the finished message.
    linda:send('finished', true)

    -- Wait for the thread to exit.
    p4result:join()
</pre>





### p4.maxlocktime -> int

Limit the amount of time (in milliseconds) spent during data scans to prevent the server 
from locking tables for too long. Commands that take longer than the limit will be 
aborted. The limit remains in force until you disable it by setting it to zero. See `p4 help 
maxlocktime` for information on the commands that support this limit.





### p4.maxresults -> int

Limit the number of results Perforce permits for subsequent commands. Commands that 
produce more than this number of results will be aborted. The limit remains in force until 
you disable it by setting it to zero. See `p4 help maxresults` for information on the 
commands that support this limit.





### p4.maxscanrows -> int

Limit the number of database records Perforce scans for subsequent commands. 
Commands that attempt to scan more than this number of records will be aborted. The 
limit remains in force until you disable it by setting it to zero. See `p4 help maxscanrows` 
for information on the commands that support this limit.





### p4.os -> string

Returns one of `UNIX`, `vms`, `NT`, `Mac`, or `nil`.





### p4.output -> list (read-only)

Contains the array of output messages (either string or tables) that arose during execution of
the last command.

<pre>
    require 'p4'

    p4 = P4.P4()
    pcall(function()
        p4:connect()
        files = p4:run_sync()
    end)

    for _, o in ipairs(p4.output) do
        print(o.clientFile)
    end

    p4:disconnect()
</pre>




### p4.p4config_file -> string (read-only)

Contains the name of the current `P4CONFIG` file, if any. This attribute cannot be set.





### p4.password -> string

Contains your Perforce password or login ticket. If not used, takes the value of `P4PASSWD` 
from any `P4CONFIG` file in effect, or from the environment according to the normal 
Perforce conventions. 

This password is also used if you later call `p4:run_login()` to log in using the 2003.2 and 
later ticket system. After running `p4:run_login()`, the attribute contains the ticket the 
allocated by the server.

<pre>
    require 'p4'
    p4 = P4.P4()
    p4.password = "mypass"
    p4:connect()
    p4:run_login()
</pre>






### p4.port -> string

Contains the host and port of the Perforce server to which you want to connect. It defaults 
to the value of `P4PORT` in any `P4CONFIG` file in effect, and then to the value of `P4PORT` taken 
from the environment.

<pre>
    require 'p4'
    p4 = P4.P4()
    p4.port = "localhost:1666"
    p4:connect()
    ...
</pre>





### p4.prog -> string

Contains the name of the program, as reported to Perforce system administrators running 
`p4 monitor show -e`. The default is `unnamed p4lua script`.

<pre>
    require 'p4'
    p4 = P4.P4()
    p4.prog = "sync-script"
    print( p4.prog )
    p4:connect()
</pre>






### p4.server_level -> int (read-only)

Returns the current Perforce server level. Each iteration of the Perforce Server is given a 
level number. As part of the initial communication this value is passed between the client 
application and the Perforce Server. This value is used to determine the communication 
that the Perforce Server will understand. All subsequent requests can therefore be tailored 
to meet the requirements of this Server level. 

This attribute is 0 before the first command is run, and is set automatically after the first 
communication with the server. 

For the API integer levels that correspond to each Perforce release, see:
<http://kb.perforce.com/?article=571>





### p4.tagged -> int

If `true`, `p4.tagged` enables tagged output. By default, tagged output is on.

<pre>
    require 'p4'
    p4 = P4.P4()
    p4.tagged = false
    print(p4.tagged)
</pre>




### p4.ticket_file -> string

Contains the location of the `P4TICKETS` file





### p4.user -> string

Contains the Perforce username. It defaults to the value of `P4USER` taken from any 
`P4CONFIG` file present, or from the environment as per the usual Perforce convention.

<pre>
    require 'p4'
    p4 = P4.P4()
    p4.user = "sven"
    p4:connect()
    ...
    p4:disconnect()
</pre>





### p4.version -> string

Contains the version of the program, as reported to Perforce system administrators in the 
server log. 

<pre>
    require 'p4'
    p4 = P4.P4()
    p4.version = "123"
    print(p4.version)
    p4:connect()
</pre>






### p4.warnings -> list (read-only)

Contains the array of warnings that arose during execution of the last command 

<pre>
    require 'p4'

    p4 = P4.P4()
    pcall(function()
        p4:connect()
        p4.exception_level = 2 -- File(s) up-to-date warning
        files = p4:run_sync()
    end)

    for _, w in ipairs(p4.warnings) do
        print(w)
    end

    p4:disconnect()
</pre>








## Class Methods

### P4.P4()

Construct a new P4 object. For example: 

<pre>
    require 'p4'
	p4 = P4.P4()
</pre>





### P4.identify()

Return the version of P4 that you are using.

<pre>
lua -e "require 'p4'; print(P4.identify())"
</pre>





## Instance Methods

### p4:connect()

Initializes the Perforce client and connects to the server.

If the connection is successfully established, returns `true`. If the connection fails and 
`exception_level` is `0`, returns `nil` and an error message, otherwise raises an exception. If already 
connected, returns `true` and a message.

<pre>
    require 'p4'
	p4 = P4.P4()
	p4:connect()
</pre>






### p4:connected() -> boolean

Returns `true` if connected to the Perforce Server and the connection is alive, otherwise 
`false`.

<pre>
    require 'p4'
	p4 = P4.P4()
    print(p4.connected())
    p4:connect()
    print p4:connected()
</pre>






### p4:delete_*&lt;spectype&gt;*( [ options ], name ) -> list

The <tt>delete_<em>spectype</em></tt> methods are shortcut methods that allow you to delete the 
definitions of clients, labels, branches, etc. These methods are equivalent to:

<pre>
    p4:run( &lt;spectype&gt;, '-d', [options], &lt;spec name&gt; )
</pre>

The following code uses `delete_client` to delete client workspaces that have not been 
accessed in more than 365 days: 

<pre>
    require 'p4'

    p4 = P4.P4()
    now = os.time()

    pcall(function()
        p4:connect()
        for i, client in ipairs(p4:run_clients()) do
            -- If the client has not been accessed for a year, delete it
            if client.Access + 365 * 24 * 60 * 60 < now then
                print("Deleting client " .. client.client)
                p4:delete_client( '-f', client[ "client" ] )
            end
        end
    end)

    for i, e in ipairs(p4.errors) do
        print(e)
    end

    p4:disconnect()
</pre>

This code deletes all labels:

<pre>
    require 'p4'
    p4 = P4.P4()
    p4:connect()
    for i, label in ipairs(p4:run_labels()) do
        print("Deleting label " .. label.label)
        p4:delete_label( '-f', label.label )
    end
    p4:disconnect()
</pre>



### p4:disconnect()

Disconnect from the Perforce Server. Call this method before exiting your script. 

<pre>
    require 'p4'
	p4 = P4.P4()
	p4:connect()
	...
	p4:disconnect()
</pre>






### p4:env( var, [value] )

If `value` isn't specified, get the value of a Perforce environment variable, taking into account
`P4CONFIG` files and (on Windows) the registry.

<pre>
    require 'p4'
	p4 = P4.P4()
	print( p4:env( "P4PORT" ) )
</pre>

If `value` is specified, sets the Perforce environment variable `var` with `value`.

<pre>
    require 'p4'
    p4 = P4.P4()
    p4:env( 'P4CONFIG', 'p4.ini' )
</pre>





### p4:fetch_*&lt;spectype&gt;*() -> P4.Spec

The <tt>fetch_<em>spectype</em></tt> methods are shortcuts for running

<pre>
    p4:run("spectype", "-o")[1].
</pre>

For example: 

<pre>
    label = p4:fetch_label(labelname)
    change = p4:fetch_change(changeno)
    clientspec = p4:fetch_client(clientname)
</pre>

are equivalent to

<pre>
    label = p4:run("label", "-o", labelname)[1]
    change = p4:run("change", "-o", changeno)[1]
    clientspec = p4:run("client", "-o", clientname)[1]
</pre>






### p4:format_spec( &lt;spectype&gt;, table ) -> string

Converts the fields in the table containing the elements of a Perforce form (spec) into the 
string representation familiar to users. The first argument is the type of spec to format: for 
example, `client`, `branch`, `label`, and so on. The second argument is the hash to parse.

There are shortcuts available for this method. You can use `p4:format_spectype( table )` instead of
`p4:format_spec( spectype, table )` where `spectype` is the name of a Perforce spec, such as
`client`, `label`, etc.





### p4:format_*&lt;spectype&gt;*( table ) -> string

The <tt>format_<em>spectype</em></tt> methods are shortcut methods that allow you to quickly fetch the 
definitions of clients, labels, branches, etc. They're equivalent to:

<pre>
    p4:format_spec( spectype, table )
</pre>






### p4:parse_spec( &lt;spectype&gt;, string ) -> P4.Spec

Parses a Perforce form (spec) in text form into a Lua table using the spec definition 
obtained from the server. The first argument is the type of spec to parse: `client`, `branch`, 
`label`, and so on. The second argument is the string buffer to parse.

There are shortcuts available for this method. You can use `p4:parse_spectype( buf )` instead of
`p4:parse_spec( spectype, buf )` where spectype is one of `client`, `branch`, `label`, and so on.






### p4:parse_*&lt;spectype&gt;*( string ) -> P4.Spec

This is equivalent to `parse_spec( spectype, string )`. 

For example, `parse_job(myJob)` converts the String representation of a job spec into a 
Spec object.

To parse a spec, P4 needs to have the spec available. When not connected to the Perforce 
Server, P4 assumes the default format for the spec, which is hardcoded. This assumption 
can fail for jobs if the Server’s jobspec has been modified. In this case, your script can load 
a job from the Server first with the command `fetch_job(’somename’)`, and P4 will cache 
and use the spec format in subsequent `parse_job()` calls.






### p4:run(cmd, [arg, ...])

Base interface to all the run methods in this API. Runs the specified Perforce command 
with the arguments supplied. Arguments may be in any form as long as they can be 
converted to strings by `tostring()`.

The `p4:run()` method returns a list of results whether the command succeeds or fails; the 
list may, however, be empty. Whether the elements of the array are strings or 
dictionaries depends on

(a) server support for tagged output for the command, and

(b) whether tagged output was disabled by calling `p4.tagged = false`.

In the event of errors or warnings, and depending on the exception level in force at the 
time, `run()` raises an exception. If the current exception level is below the threshold for 
the error/warning, `run()` returns the output as normal and the caller must explicitly 
review `p4.errors` and `p4.warnings` to check for errors or warnings.

<pre>
    require 'p4'
	p4 = P4.P4()
    p4:connect()
    spec = p4:run( "client", "-o" )[1]
    p4:disconnect()
</pre>

Shortcuts are available for `p4:run`. For example, `p4:run_command( args )` is equivalent 
to `p4:run( "command", args )`.

There are also some shortcuts for common commands such as editing Perforce forms and 
submitting. For example, this:

<pre>
    require 'p4'
	p4 = P4.P4()
    p4:connect()
    clientspec = p4:run_client( "-o" )[1]
    clientspec.Description = "Build client"
    p4.input = clientspec
    p4:run_client( "-i" )
    p4:disconnect()
</pre>

...may be shortened to

<pre>
    require 'p4'
	p4 = P4.P4()
    p4:connect()
    clientspec = p4:fetch_client()
    clientspec.Description = "Build client"
    p4:save_client( clientspec )
    p4:disconnect()
</pre>

The following are equivalent:

<table>
	<tr><th>Shortcut</th><th>Equivalent to</th></tr>
	<tr><td><tt>p4:delete_spectype</tt></td><td><tt>p4:run( "spectype", "-d ")</tt></td></tr>
	<tr><td><tt>p4:fetch_spectype</tt></td><td><tt>p4:run( "spectype", "-o ")[1]</tt></td></tr>
	<tr><td><tt>p4:save_spectype( spec )</tt></td><td><tt>p4.input = spec; p4:run( "spectype", "-i" )</tt></td></tr>
</table>

As the commands associated with `fetch_spectype` typically return only one item, these 
methods do not return an array, but instead return the first result element.

For convenience in submitting changelists, changes returned by `fetch_change()` can be 
passed to `run_submit()`. For example:

<pre>
    require 'p4'
	p4 = P4.P4()
	p4:connect()
	spec = p4:fetch_change()
	spec.Description = "Automated change"
	p4:run_submit(spec)
	p4:disconnect()
</pre>





### p4:run{cmd, [arg, ...]}

`p4:run()`, as described above, performs the Perforce operation, collecting all output, warnings, and errors, and then returns that information to the user.  It is suited for short-run commands or those where user interface is not an issue.  Additionally, for large operations, exorbitant amounts of memory may be used by the mass collection process.

The table constructor form of `p4:run{}` allows for all the functionality above but with the ability to patch into and receive information from the output, warning, or error streams as it becomes available.

P4 command arguments are specified as in `p4:run()`.  Additional table constructor function entries are as follows:

* `OutputText( info )` - Called whenever regular output is received, generally a string or table.
* `OutputWarning( info )` - Called whenever a warning is received.
* `OutputError( info )` - Called whenever an error is received.

If a function is not specified, the related `p4.output`, `p4.warnings`, or `p4.errors` tables continue to fill.  If this is not desirable, specify the function and make it empty.

<pre>
    require 'p4'
	p4 = P4.P4()
	p4:connect()
	p4:run{ 'sync',
	    OutputText = function(info)
		    print( 'Synced: ' .. info.depotFile )
		end,
		OutputWarning = function(info)
		    print(info)
			table.insert(p4.warnings, info)
		end
	}
	p4:disconnect()
</pre>







### p4:run_cmd([arg, ...])

Shorthand for `p4:run("cmd", arguments... )`.






### p4:run_filelog(&lt;fileSpec&gt;) -> list

Runs a `p4 filelog` on the `fileSpec` provided and returns an array of `P4.DepotFile` 
results (when executed in tagged mode), or an array of strings when executed in 
nontagged mode. By default, the raw output of `p4 filelog` is tagged; this method 
restructures the output into a more user-friendly (and object-oriented) form. 

For example:

<pre>
    require 'p4'
    p4 = P4.P4()
    p4:connect()
    files = p4:run_filelog('index.html')
    print(files[1])
    for _, r in ipairs(files[1].revisions) do
        print(r)
        for _, i in ipairs(r.integrations) do
            print(i)
        end
    end
</pre>






### p4:run_login( arg... ) -> list

Runs `p4_login` using a password (or other arguments) set by the user.






### p4:run_password( oldpass, newpass ) -> list

A thin wrapper to make it easy to change your password. This method is (literally) 
equivalent to the following: 

<pre>
    p4:input( [ oldpass, newpass, newpass ] )
    p4:run( "password" )
</pre>

For example

<pre>
    require 'p4'
    p4 = P4.P4()
    p4.password = "myoldpass"

    pcall(function()
        p4:connect()
        p4:run_password( "myoldpass", "mynewpass" )
    end)

    for i, e in ipairs(p4.errors) do
        print(e)
    end

    p4:disconnect()
</pre>






### p4:run_resolve( [resolver], [arg...] ) -> list

Run a `p4 resolve` command. Interactive resolves require the resolver parameter to be 
an object of a class derived from `P4.Resolver`. In these cases, the resolve method of this 
class is called to handle the resolve. For example:

<pre>
    p4:run_resolve()
</pre>

To perform an automated merge that skips whenever conflicts are detected: 

<pre>
    require 'p4'
    p4 = P4.P4()
    p4:connect()

    MyResolver = {}
    function MyResolver.resolve(self, mergeData)
        print(mergeData.your_name, mergeData.their_name, mergeData.base_name,
            mergeData.your_path, mergeData.their_path, mergeData.base_path,
            mergeData.result_path, mergeData.merge_hint)
        if not mergeData.merge_hint == "e" then
            return mergeData.merge_hint
        else
            return "s" -- skip the resolve, there is a conflict
		end
    end

    p4:run_resolve { resolver = MyResolver }

    p4:disconnect()
</pre>

In non-interactive resolves, no `P4.Resolver` object is required. For example: 

<pre>
    p4:run_resolve ( "-at" )
</pre>





### p4:run_submit( [ hash ], [ arg... ] ) -> list

Submit a changelist to the server. To submit a changelist, set the fields of the changelist as 
required and supply any flags:

You can also submit a changelist by supplying the arguments as you would on the 
command line:

<pre>
    change = p4:fetch_change()
    change._description = "Some description"
    p4:run_submit( "-r", change )
    p4:run_submit( "-d", "Some description", "somedir/..." )
</pre>






### p4:save_&lt;spectype&gt;()

The `save_spectype` methods are shortcut methods that allow you to quickly update the 
definitions of clients, labels, branches, etc. They are equivalent to: 

<pre>
    p4.input = dictOrString
    p4:run( spectype, "-i" )
</pre>

For example:

<pre>
    require 'p4'
    p4 = P4.P4()

	pcall(function()
        p4:connect()
        client = p4:fetch_client()
        client.Owner = p4.user
        p4:save_client( client )
	end)

    for i, e in ipairs(p4.errors) do
        print e
	end

    p4:disconnect()
</pre>




-------------------------
# Class P4.DepotFile
## Description

Utility class providing easy access to the attributes of a file in a Perforce depot. Each 
`P4.DepotFile` object contains summary information about the file and a list of revisions 
(`P4.Revision` objects) of that file. Currently, only the `P4:run_filelog` method returns a 
list of `P4.DepotFile` objects.




## Instance Attributes

### df.depotFile -> string 

Returns the name of the depot file to which this object refers. 

### df.revisions -> list 

Returns a list of P4.Revision objects, one for each revision of the depot file. 

## Class Methods

None.

## Instance Methods

None.



-------------------------
# Class P4.Revision

## Description

Utility class providing easy access to the revisions of `P4.DepotFile` objects. Created by 
`P4:run_filelog()`.

## Instance Attributes

### rev.action -> string 

Returns the name of the action which gave rise to this revision of the file. 

### rev.change -> int

Returns the change number that gave rise to this revision of the file. 

### rev.client -> string 

Returns the name of the client from which this revision was submitted. 

### rev.depotFile -> string

Returns the name of the depot file to which this object refers.

### rev.desc -> string 

Returns the description of the change which created this revision. Note that only the first 
31 characters are returned unless you use `p4 filelog -L` for the first 250 characters, or `p4 
filelog -l` for the full text.

### rev.digest -> string 

Returns the MD5 checksum of this revision.

### rev.fileSize -> string

Returns this revision’s size in bytes. 

### rev.integrations -> list

Returns the list of P4.Integration objects for this revision. 

### rev.rev -> int

Returns the number of this revision of the file.

### rev.time -> datetime 

Returns the date/time that this revision was created. 

### rev.type -> string

Returns this revision’s Perforce filetype. 

### rev.user -> string 

Returns the name of the user who created this revision. 

## Class Methods

None.

## Instance Methods

None.




-----------------------------
# Class P4.Integration

## Description

Utility class providing easy access to the details of an integration record. Created by 
`P4:run_filelog()`.

## Instance Attributes

### integ.how -> string

Returns the type of the integration record - how that record was created. 

### integ.file -> string 

Returns the path to the file being integrated to/from. 

### integ.erev -> int 

Returns the end revision number used for this integration. 

### integ.srev -> int 

Returns the start revision number used for this integration. 

## Class Methods

None.

## Instance Methods

None.



---------------------------------
# Class P4.Map

## Description

The `P4.Map` class allows users to create and work with Perforce mappings, without 
requiring a connection to a Perforce server.

## Instance Attributes

None

## Class Methods

### P4.Map( [ table ] ) -> P4.Map

Constructs a new `P4.Map` object.





### P4.Map_join ( map1, map2 ) -> P4.Map

Join two `P4.Map` objects and create a third. 

The new map is composed of the left-hand side of the first mapping, as joined to the right-
hand side of the second mapping. For example:

<pre>
    -- Map depot syntax to client syntax
    client_map = P4.Map()
    client_map:insert( "//depot/main/...", "//client/..." )
    -- Map client syntax to local syntax
    client_root = P4.Map()
    client_root:insert( "//client/...", "/home/tony/workspace/..." )
    -- Join the previous mappings to map depot syntax to local syntax
    local_map = P4.Map_join( client_map, client_root )
    local_path = local_map:translate( "//depot/main/www/index.html" )
    # local_path is now /home/tony/workspace/www/index.html
</pre>





## Instance Methods

### map:clear()

Empty a map.





### map:count() -> int

Return the number of entries in a map.




### map:is_empty() -> boolean

Test whether a map object is empty.





### map:insert( string ... )

Inserts an entry into the map. 

May be called with one or two arguments. If called with one argument, the string is 
assumed to be a string containing either a half-map, or a string containing both halves of 
the mapping. In this form, mappings with embedded spaces must be quoted. If called 
with two arguments, each argument is assumed to be half of the mapping, and quotes are 
optional.

<pre>
    -- called with two arguments:
    map:insert( "//depot/main/...", "//client/..." )
    -- called with one argument containing both halves of the mapping:
    map:insert( "//depot/live/... //client/live/..." )
    -- called with one argument containing a half-map:
    -- This call produces the mapping "depot/... depot/..."
    map:insert( "depot/..." )
</pre>





### map:translate ( string, [ boolean ] )-> string

Translate a string through a map, and return the result. If the optional second argument is 
1, translate forward, and if it is 0, translate in the reverse direction. By default, translation 
is in the forward direction.





### map:includes( string ) -> boolean

Tests whether a path is mapped or not. 

<pre>
    if map:includes( "//depot/main/..." ) then
        ...
</pre>




### map:reverse() -> P4.Map

Return a new `P4.Map` object with the left and right sides of the mapping swapped. The 
original object is unchanged.





### map:lhs() -> list

Returns the left side of a mapping as an array.





### map:rhs() -> list

Returns the right side of a mapping as an array.





### map:as_array() -> list

Returns the map as an array.






-------------------------------
# Class P4.MergeData
## Description 

Class containing the context for an individual merge during execution of a `p4 resolve`. 

## Instance Attributes

### md.your_name -> string 

Returns the name of “your” file in the merge. This is typically a path to a file in the 
workspace. 



### md.their_name -> string

Returns the name of “their” file in the merge. This is typically a path to a file in the depot. 

### md.base_name -> string 

Returns the name of the “base” file in the merge. This is typically a path to a file in the 
depot. 

### md.your_path -> string 

Returns the path of “your” file in the merge. This is typically a path to a file in the 
workspace. 

### md.their_path -> string 

Returns the path of “their” file in the merge. This is typically a path to a temporary file on 
your local machine in which the contents of their_name have been loaded. 

### md.base_path -> string 

Returns the path of the base file in the merge. This is typically a path to a temporary file 
on your local machine in which the contents of base_name have been loaded. 

### md.result_path -> string 

Returns the path to the merge result. This is typically a path to a temporary file on your 
local machine in which the contents of the automatic merge performed by the server have 
been loaded. 

### md.merge_hint -> string 

Returns the hint from the server as to how it thinks you might best resolve this merge. 



## Instance Methods

### md.run_merge() -> boolean

If the environment variable `P4MERGE` is defined, run_merge() invokes the specified 
program and returns a boolean based on the return value of that program.



-----------------------------------
# Class P4.Resolver

## Description

`P4.Resolver` is a class for handling resolves in Perforce. It is intended to be subclassed, 
and for subclasses to override the `resolve()` method. When `P4:run_resolve()` is called 
with a `P4.Resolver` object, it calls the `resolve()` method of the object once for each 
scheduled resolve.

## Instance Attributes

None

## Class Methods

None

## Instance Methods

### resolver.resolve(self, mergeData) -> string 

Returns the resolve decision as a string. The standard Perforce resolve strings apply: 

<table>
  <tr><th>String</th><th>Meaning</th></tr>
  <tr><td><tt>ay</tt></td><td>Accept Yours</td></tr>
  <tr><td><tt>at</tt></td><td>Accept Theirs</td></tr>
  <tr><td><tt>am</tt></td><td>Accept Merge result</td></tr>
  <tr><td><tt>ae</tt></td><td>Accept Edited result</td></tr>
  <tr><td><tt>s</tt></td><td>Skip the merge</td></tr>
  <tr><td><tt>q</tt></td><td>Abort the merge</td></tr>
</table>
  
By default, all automatic merges are accepted, and all merges with conflicts are skipped. 
The `resolve` method is called with a single parameter, which is a reference to a 
`P4.MergeData` object.



-------------------------
# Class P4.Spec

## Description

Utility class providing easy access to the attributes of the fields in a Perforce form. 
Only valid field names may be set in a `P4.Spec` object. Only the field name is validated, 
not the content. Attributes provide easy access to the fields. 

## Instance Attributes

### spec._&lt;fieldname&gt; -> string

Contains the value associated with the field named &lt;fieldname&gt;. 




### spec.permitted_fields -> dict 

Contains an array containing the names of fields that are valid in this spec object. This 
does not imply that values for all of these fields are actually set in this object, merely that 
you may choose to set values for any of these fields if you want to. 




## Class Methods

### P4.Spec.new( dict ) -> P4.Spec 

Constructs a new `P4.Spec` object given an array of valid fieldnames. 




## Instance Methods

None.




