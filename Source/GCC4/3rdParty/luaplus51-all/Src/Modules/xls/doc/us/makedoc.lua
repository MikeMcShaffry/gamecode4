local cosmo = require "cosmo"
require "markdown"

local pages = {
  { name = "Home", file = "index", sections = {} },
  { name = "License", file = "license", sections = {} }
}

local project = {
  name = "xls",
  blurb = "Read/write xls files",
--  logo = "orbit.png",
}

local template = [==[

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
   "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
	<title>$name</title>
    <link rel="stylesheet" href="doc.css" type="text/css"/>
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
</head>

<body>

<div id="container">

<div id="product">

	<div id="product_name"><big><strong>$name</strong></big></div>
	<div id="product_description">$blurb</div>
</div> <!-- id="product" -->

<div id="main">

<div id="navigation">
<h1>$name</h1>
	<ul>
$pages[[
		<li>$namelink
			<ul>
                            $sections[=[<li><a href="$anchor">$name</a></li>]=]
			</ul>
		</li>
]]
	</ul>
</div> <!-- id="navigation" -->

<div id="content">

$content

</div> <!-- id="content" -->

</div> <!-- id="main" -->

<div id="about">
	<p><a href="http://validator.w3.org/check?uri=referer">Valid XHTML 1.0!</a></p>
</div> <!-- id="about" -->

</div> <!-- id="container" -->

</body>
</html>

]==]

local function readfile(filename)
  local file = io.open(filename)
  local contents = file:read("*a")
  file:close()
  return contents
end

local function writefile(filename, contents)
  local file = io.open(filename, "w+")
  file:write(contents)
  file:close()
end

local function gen_page(project, pages, p)
  project.pages = function ()
    for _, page in ipairs(pages) do
      local namelink
      if page.file == p.file then
        namelink = cosmo.fill([[<strong>$name</strong>]], { name = page.name})
      else
        namelink = cosmo.fill([[<a href="$file.html">$name</a>]], { name = page.name, file = page.file})
      end
      cosmo.yield{ namelink = namelink, sections = function ()
                                                     for _, s in ipairs(page.sections) do
                                                       cosmo.yield{ name = s.name, anchor =
                                                         page.file .. ".html#" .. s.anchor }
                                                     end
                                                   end }
    end
  end
  return (cosmo.fill(template, project))
end

for _, p in ipairs(pages) do
  project.content = markdown(readfile(p.file .. ".md"))
  writefile(p.file .. ".html", gen_page(project, pages, p))
end
