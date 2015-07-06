#!/usr/bin/lua

require("mixlua")

-- Example : This is the table who contain data to save
ex = {
	name = "Example 1",
	width = 1024,
	height = 768,
	windows = {
		{name = "Win 1", level = 3},
		{name = "Win 2", level = 1},
		{name = "Win 3", level = 5},
		{name = "Win 4", level = 4},
	}
}

-- This is a first template for saving in file like ini configuration files
template_ini = [[
[general]
name   = <<= ex.name   >>
width  = <<= ex.width  >>
height = <<= ex.height >>

<< for id, win in ipairs(ex.windows) do >>
[window<<=id>>]
name  = <<= win.name >>
level = <<= win.level >>
<< end >>
]]

assert(mix.loadstring(template_ini, "<<", ">>"))()

-- And here a template to save it in an xml file
template_xml = [[
<?xml version="1.0">
<config>
  <general>
    <name>  $$<- ex.name   $$</name>
    <width> $$<- ex.width  $$</width>
    <height>$$<- ex.height $$</height>
  </general>
  $$ for id, win in ipairs(ex.windows) do $$
    <window id="$$<-id$$">
      <name> $$<- win.name  $$</name>
      <level>$$<- win.level $$</level>
    </window>
  $$ end $$
</config>
]]

assert(mix.loadstring(template_xml, "$$", "$$", "<-"))()

