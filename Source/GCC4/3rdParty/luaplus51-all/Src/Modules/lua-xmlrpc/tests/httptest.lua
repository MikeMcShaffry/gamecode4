---------------------------------------------------------------------
-- XML-RPC Automatic Tests
-- See Copyright Notice in license.html
-----------------------------------------------------------------------

require"xmlrpc.http"

local function init_launcher(launcher)
  if launcher == 'cgilua' then 
    require"cgilua"
    cgilua.htmlheader()
    io.write = function(str)
      cgilua.put(string.gsub(str, '\n','<br>'))
    end --io.write
  end --if 
end

local function dumpt(t)
  for k,v in pairs(t) do
    io.write('\nk='..tostring(k)..'\tv='..tostring(v))
    if type(v) == 'table' then dumpt(v) end
  end --for
end

local function test(url, rtype, rvalue, method, args)
  local status, res
  io.write('\nURL='..url..'\nMETHOD='..method..'\nPARAMETERS=')
  io.write(unpack(args or {'void'}))
  if args then
    status, xmlstatus, res = pcall( xmlrpc.http.call,url, method, unpack(args))
  else
    status, xmlstatus, res = pcall( xmlrpc.http.call,url, method)  
  end --if
  if not status then
    io.write('\nFAILURE! '..xmlstatus)
  else
    if not xmlstatus and rtype ~= 'failure' then
      io.write('\nFAILURE! '..res)
    else
      io.write('\nreturn type expected= '..rtype)  
      if type(res) == rtype or (not xmlstatus and rtype == 'failure') then
        io.write(' (OK)')
      else
        io.write(' (FAILURE) returning a '..type(res))
      end --if    
      if rvalue then
        io.write('\nvalue expected='..tostring(rvalue))
        if type(res) == 'table' then
          dumpt(res)
        elseif tostring(rvalue) == tostring(res) then
          io.write(' (OK)')
        else
          io.write(' (FAILURE) value returned='..tostring(res))
        end --if
      end --if
    end --if
  end --if
  io.write('\n')
end

function syndic8()
  test( "http://www.syndic8.com/xmlrpc.php", "string", nil, "syndic8.GetFeedCount" )
end

local server = 'http://localhost:80/server.lua'

function calc()
  test( server, 'number', '25', 'calc.add', {10,15} )
  test( server, 'string', '0.1v', 'calc.getVersion' )
  test( server, 'boolean', 'true', 'calc.isprime', {7} )
  test( server, 'boolean', 'false', 'calc.isprime', {80} )
  test( server, 'table', {{1,0},{0,1}}, 'calc.MID' )
  test( server, 'failure', 'Unavailable method', 'calc' )
  test( server, 'failure', 'Unavailable method', 'calc.MID2' )
  test( server, 'failure', 'Unavailable method', 'foo', {2} )
  test( server, 'failure', 'Unavailable method', 'foo.foo2.foo3', {3,4,5} )
--  test( 'http://localhost:82/calc.xmlrpc', 'number', '25', 'calc.add', {10,15} )
end

local launcher = {
  'lua',
  'cgilua', 
}

local tests = {  
  [1] = { name='SYNDIC8', func=syndic8 },
  [2] = { name='CALCULATOR', func=calc },
  }
  
init_launcher(launcher[1])
  
io.write(
[[
LuaXMLRPC - Automatic Tests
0.1v
]]
)  
    
for k,v in pairs( tests ) do
  io.write( "\n\nT"..k.." - "..v.name.."\n" )
  v.func()
end --for
