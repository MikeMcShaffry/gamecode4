#!/usr/bin/env cgilua.cgi

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
   "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
    <title>CGILua Test</title>
    <link rel="stylesheet" href="css/doc.css" type="text/css"/>
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
    <%
    require"cgilua.cookies"
    if cgilua.POST.user then
        cgilua.cookies.sethtml("cookie_kepler", cgilua.POST.user)
    end
    %>
</head>

<body>

<div id="container">

<div id="product">
	<div id="product_logo"><a href="http://www.keplerproject.org">
		<img src="img/keplerproject.gif" alt="Kepler Project logo" /></a>
    </div>
	<div id="product_name"><big><strong></strong></big></div>
	<div id="product_description">CGILua simple tests</div>
</div> <!-- id="product" -->

<div id="main">

<div id="navigation">
<% 
local mods = {}
for file in lfs.dir("doc") do
    local attr = lfs.attributes ("doc/"..file)
    if attr.mode == "directory" and file ~= "." and file ~= ".." then
        table.insert(mods, file)
    end
end
table.sort(mods)
%>

<h1>Kepler</h1>
	<ul>
        <li><a href="index.lp">Documentation</a>
            <ul>
                <%
                for _,file in ipairs(mods) do
                %>
                <li><strong><a href="doc/<%= file %>/index.html"><%= file %></a></strong></li>
                <%
                end
                if not next(mods) then
                %>
                <li><strong>(None)</strong></li>
                <%
                end
                %>
            </ul>
        </li>
        <li><strong>Tests</strong></li>
	</ul>
</div> <!-- id="navigation" -->

<div id="content">


<h2>Form Handling</h2>

<p>
Entering values on this form should display them as values in the first submission,
and as a cookie in the next submission
</p>

<form method="post" action="test.lp">
    <label>User name: </label><input name="user" maxlength="20" size="20">
    <label>Password: </label><input name="pass" type="password" maxlength="20" size="20">
    <input type="submit" value="Post it">
    <input type="reset" value="Reset">
</form>

<p>
The values should show the previous POST
</p>

<p>
Values: Username = <%= cgilua.POST.user or "(not set)"%>, Password = <%= cgilua.POST.pass or "(not set)"%>
</p>

<h2>Cookies test</h2>

<p>
Here you should see the values posted before the ones shown above
</p>

<p>
cookie_kepler = <%= cgilua.cookies.get("cookie_kepler") or "(not set)" %>
</p>

<h2>File Upload</h2>

<p>Choose a file to upload, press "Upload" and a link to the file should
appear below with the corresponding "Remove" button.</a>
<form method="POST" enctype="multipart/form-data" action="test.lp">
    <input type="file" name="file">
    <input type="submit" value="Upload">
</form>

<% 
    local f = cgilua.POST.file
    if f and next(f) then
        local _, name = cgilua.splitonlast(f.filename)
        local file = f.file
        local dest = io.open(name, "wb")
        if dest then
            local bytes = file:read("*a")
            dest:write(bytes)
            dest:close()
            cgilua.print("<a href='"..name.."'>"..name.."</a>\n")
            cgilua.print([[<form method="POST" enctype="multipart/form-data" action="test.lp">]])
            cgilua.print([[<input type="hidden" name="filename" value="]]..name..[[">]])
            cgilua.print([[<input type="submit" name="remove" value="Remove">]])
            cgilua.print([[</form>]])
        end
    end
%>

<%
    if cgilua.POST.remove then
        os.remove(cgilua.POST.filename)
    end
%>

<%
local function showtable(t)
    cgilua.put "{"
    for i,v in pairs (t) do
        cgilua.put("\n")
        if type(v) == "table" then
            local vv = "{\n"
            for a,b in pairs(v) do
                vv = string.format ("%s  %s = [[%s]],\n", vv, a, tostring(b))
            end
            v = vv.." },"
            cgilua.put (string.format (" %s = %s", i, tostring(v)))
        else
            cgilua.put (string.format (" %s = [[%s]],", i, tostring(v)))
        end
    end
    if next(t) then
        cgilua.put "\n"
    end
    cgilua.put "}\n"
end
%>

<h2>cgilua.QUERY</h2>

<pre class="example">
<% showtable (cgilua.QUERY) %>
</pre>

<h2>cgilua.POST</h2>
<pre class="example">
<% showtable (cgilua.POST) %>
</pre>

<h2>CGILua Variables</h2>

<table border="1">
<%
local vars = {
    "script_file", "script_path", "script_pdir", "script_vdir", "script_vpath", "urlpath",
}
for _, v in ipairs(vars) do %>
  <tr><td>cgilua.<%= v %></td><td><%= tostring(cgilua[v]) %></td></tr>
<% end %>
</table>


<h2>Server Variables</h2>

<table border="1">
<%
local vars = {
    "SERVER_SOFTWARE", "SERVER_NAME", "SERVER_PROTOCOL", "SERVER_PORT",
    "GATEWAY_INTERFACE", "REQUEST_METHOD",
    "SCRIPT_NAME", "PATH_INFO", "PATH_TRANSLATED", "QUERY_STRING",
    "CONTENT_TYPE", "CONTENT_LENGTH", 
    "REMOTE_ADDR", "REMOTE_HOST", "REMOTE_USER", "REMOTE_IDENT",
    "AUTH_TYPE",
}
for _, v in ipairs(vars) do %>
  <tr><td><%= v %></td><td><%= tostring(cgilua.servervariable(v)) %></td></tr>
<% end %>
</table>

<h2>Multiple Output</h2>

<p>
The next test should show numbers, from 1 to 3, and the string "OK" together.
The first line should show the results without spaces and the second should separate them with tabs.
</p>
  	 
<pre class="example">
cgilua.put(1, 2, 3, "OK")   --> <% cgilua.put(1, 2, 3, "OK") %>
</pre>

<p>The next test should show numbers, from 1 to 3, and the string "OK" separated by tabs.</p>
  	 
<pre class="example">
cgilua.print(1, 2, 3, "OK") --> <% cgilua.print(1, 2, 3, "OK") %>
</pre>

<h2>Date</h2>

<p>Today is: <%= os.date() %></p>

<h2>Image test</h2>

<p>Here should be a small image: <img src="img/test.jpg" alt="a small photograph" /></p>

<h2>FileSystem test</h2>

<p>
<%
  local d = lfs.currentdir () or ""
  cgilua.put("Iterating over "..d.."<br />")
  for file in lfs.dir(d) do cgilua.put("&nbsp;&nbsp;&nbsp;"..file.."<br />") end
%>
</p>

<h2>Containment test</h2>

<p>
<% if (x == nil) then x = 1 else x = x + 1 end %>
Expected value: 1, actual value: <%= x %>.
</p>


</div> <!-- id="content" -->

</div> <!-- id="main" -->

<div id="about">
	<p><a href="http://validator.w3.org/check?uri=referer">Valid XHTML 1.0</a></p>
	<p><small>$Id: test.lp,v 1.3 2008/07/03 20:34:36 carregal Exp $</small></p>
</div> <!-- id="about" -->

</div> <!-- id="container" -->
</body>
</html>
