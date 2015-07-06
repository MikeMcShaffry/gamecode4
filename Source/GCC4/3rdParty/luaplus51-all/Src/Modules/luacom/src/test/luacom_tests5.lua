-- LuaCOM test suite.

require "luacom"
assert(luacom)
local luacom = luacom

local skipped = false  -- whether tests were skipped

local teste_progid = "LuaCOM.Test"

luacom.StartLog("luacom.log")
luacom.abort_on_API_error = true

--UNUSED: Unk = luacom.GetIUnknown

local Events
local Cookie
local _obj

-- Objeto COM de teste
local Test = {}
Test.prop1 = 1
Test.prop2_value = 0
function Test:ShowDialog()
  print("ShowDialog called")
end
function Test:prop2(p1, p2, value)
  Events:evt1(value)
  Events:evt2()
  if value then
    Test.prop2_value = p1*p2*value
  else
    return Test.prop2_value/(p1*p2)
  end
end

--[[
BEGIN-UNUSED
DataConvTest = {}
do
  function DataConvTest:TestDATE(a, b)
    print(a)
    print(b)
    return b, a, a
  end

  function DataConvTest:TestArrayByVal(a)
    return {"j","k","l"}
  end

  function DataConvTest:TestArrayByRef(a)
    return {1,2}
  end

  function DataConvTest:TestBSTR(p1, p2)
    return p2, p1, p1..p2
  end

  function DataConvTest:TestVARIANT(p1, p2)
    return p1, p2, p2
  end
end
function Test:DataConvTest()
  return luacom.ImplInterface(DataConvTest, "LUACOM.Test",
                              "IDataConversionTest")
end
END-UNUSED
]]

-- Criação e registro do objeto COM de teste
local function init()
  local reginfo = {}  
  reginfo.VersionIndependentProgID = "LUACOM.Test"
  reginfo.ProgID = reginfo.VersionIndependentProgID..".1"
  reginfo.TypeLib = "test.tlb"
  reginfo.CoClass = "Test"
  reginfo.ComponentName = "LuaCOM-Test"
  reginfo.Arguments = "/Automation"
  
  local res = luacom.RegisterObject(reginfo)
  assert(res)
  
  -- creates and exposes COM proxy application object
  local COMtestobj
  COMtestobj, Events = luacom.NewObject(Test, "LUACOM.Test")
  assert(COMtestobj)
  assert(Events)
   
  Cookie = luacom.ExposeObject(COMtestobj)
  assert(Cookie)
  
  _obj = luacom.CreateObject("LUACOM.Test")
end

local function finish()
  luacom.RevokeObject(Cookie)
end

-- immutable tuple type
-- Useful for comparing lists in test cases.  --djm
local tuple = {}
setmetatable(tuple, {__call = function(class, ...)
  return setmetatable({n=select('#',...), ...}, tuple)
end})
function tuple.__eq(a,b)
  if getmetatable(a) ~= tuple or getmetatable(b) ~= tuple then
    return false
  end
  if a.n ~= b.n then
    return false
  end
  for i=1,a.n do
    if a[i] ~= b[i] then
      return false
    end
  end
  return true
end
function tuple.__tostring(a)
  local ts = {}
  for i=1,a.n do
    ts[i] = tostring(a[i])
  end
  return 'tuple(' .. table.concat(ts, ',') .. ')'
end

-- Funcoes auxiliares

local function asserteq(a, b)
  if a ~= b then
    error('FAIL: ' .. tostring(a) .. ' = ' .. tostring(b), 2)
  end
end

-- Converts table to string.  Supports nested tables.
local function table2string(table)
  if type(table) ~= "table" then
    return tostring(table)
  end

  local i, v = next(table, nil)
  local s = "{"
  local first = 1
  for i,v in pairs(table) do
    if first ~= 1 then
      s = s..", "            
    else
      first = 0
    end
    s = s..table2string(v)
  end
  s = s.."}"

  return s
end

-- Begin new test.
local number_test_n
local function number_test(start)
  number_test_n = number_test_n or 0

  if type(start) == "number" then
    number_test_n = start
  else
    number_test_n = number_test_n + 1
  end

  print("Test " .. number_test_n ..
        (type(start) == "string" and ": "..start or ""))
end
local nt = number_test


----------------------------------------------------------------------
----------------------------------------------------------------------


-- Testa tratamento de entradas incorretas
local function test_wrong_parameters()
  print("\n=======> test_wrong_parameters")

  nt(1)
  local res = pcall(luacom.CreateObject,{table = "aa"})
  assert(res == false)

  nt()
  local res = pcall(luacom.CreateObject)
  assert(res == false)

  nt()
  local res = pcall(luacom.ImplInterface)
  assert(res == false)

  nt()
  local d, obj = pcall(luacom.ImplInterface,{}, "blablabla", "blablabla")
  assert(obj == nil)

  nt()
  local d, obj = pcall(luacom.ImplInterface, {}, "InetCtls.Inet", "blablabla")
  assert(obj == nil)

  -- ProgID's inexistentes
  nt()
  local obj = luacom.CreateObject("blablabla")
  assert(obj == nil)

  nt()
  local obj = luacom.CreateObject(1)
  assert(obj == nil)

end


-- Teste simples
local function test_simple()
  print("\n=======> test_simple")

  nt(1)
  assert(type(_obj.Children) == "function")

  nt()
  assert(type(_obj.prop1) == "number")

  nt()
  _obj.prop1 = 2
  assert(_obj.prop1 == 2)

  nt()
  local t = {}
  t.TestArray1 = function(self, array)
    assert(table2string(array) == table2string({"1","2"}))
  end

  local obj = luacom.ImplInterface(t, "LUACOM.Test", "IDataConversionTest" )
  assert(obj)

  obj:TestArray1({1,2})
end


-- Teste de Implementacao de Interface via tabelas
local function test_iface_implementation()
  print("\n=======> test_iface_implementation")
  
  nt(1)

  local iface_ok = 0
  local iface = {}
  iface.evt1 = function(self)
    iface_ok = iface_ok + 1
  end

  iface.TestManyParams = function(
    self,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15
  )
    assert(p1 == 1)
    assert(p2 == 2)
    assert(p3 == 3)
    assert(p4 == 4)
    assert(p5 == 5)
    assert(p6 == 6)
    assert(p7 == 7)
    assert(p8 == 8)
    assert(p9 == 9)
    assert(p10 == 10)
    assert(p11 == 11)
    assert(p12 == 12)
    assert(p13 == 13)
    assert(p14 == 14)
    assert(p15 == 15)
  end

  local obj = luacom.ImplInterface(iface, teste_progid, "ITestEvents")
  assert(obj)

  nt()
  obj:evt1()
  assert(iface_ok == 1) 

  nt()
  obj:TestManyParams(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15)
  obj = nil

  collectgarbage()
end



-- Teste de stress
local function test_stress()
  print("\n=======> test_stress")

  nt(1)

  local iface = {}
  local iface_ok
  iface.BeforeNavigate = function (self)
    iface_ok = iface_ok + 1
  end

  local obj = luacom.ImplInterface(
      iface, "InternetExplorer.Application", "DWebBrowserEvents")
  assert(obj)

  iface_ok = 1
  for i=1,100000 do
    obj:BeforeNavigate()
  end

  nt()
  assert(i == iface_ok)
  obj = nil
  collectgarbage()
end


-- Teste de Connection Points
local function test_connection_points()
  print("\n=======> test_connection_points")

  nt(1)

  nt()
  local events = {}
  local events_ok = nil
  events.evt2 = function(self)
    events_ok = 1
  end

  local evt = luacom.ImplInterface(events, "LUACOM.Test", "ITestEvents")
  assert(evt)

  nt()
  luacom.addConnection(_obj, evt)

  nt()
  _obj.setprop2(1,1,2) 
  assert(events_ok)

  nt()
  luacom.releaseConnection(_obj)
  events_ok = nil

  nt()
  luacom.Connect(_obj, events)

  nt()
  _obj.setprop2(2,2,3)
  assert(events_ok)

  nt()
  luacom.releaseConnection(_obj)

  events = nil
  collectgarbage()
end


-- Teste de propget
local function test_propget()
  print("\n=======> test_propget")

  nt(1)
  local iface = {}
  iface.Value = 1
  local evt = luacom.ImplInterface(iface, "MsComCtlLib.Slider", "ISlider")
  if evt then
    assert(evt)
    assert(evt.Value == iface.Value)
  else
    print 'WARNING: MsComCtlLib.Slider test disabled.'
  end

  --nt()
  --local obj = luacom.CreateObject("COMCTL.TabStrip.1")
  --t = obj.Tabs
end


-- Teste de propput
local function test_propput()
  print("\n=======> test_propput")

  nt(1) -- propput simples
  assert(_obj)
  -- Propput normal
  _obj.prop1 = 1
  assert(_obj.prop1 == 1)

  nt() -- propput parametrizado
  _obj:setprop2(2,3,1.1)
  assert(_obj:prop2(2,3) == 1.1)

  nt()
  _obj:setprop2(2, 1, "1")
  assert(tonumber(_obj:getprop2(2, 1, "1")) == 1)
end



local function test_index_fb()
  print("\n=======> test_index_fb")

  nt(1)
  local tm = getmetatable(_obj)["__index"]
  assert(tm)

  nt()
  local res = pcall(tm, nil,nil)
  assert(res == false)

  nt()
  local res = pcall(tm, {}, nil)
  assert(res == false)

  nt()
  local res = pcall(tm, {}, "test")
  assert(res == false)

  nt()
  local d, res = pcall(tm, _obj, nil)
  assert(res == nil)

  nt()
  local f = tm(_obj, "blublublabla")
  assert(f == nil)

  nt()
  local f = tm(_obj, "HideDialog")
  assert(type(f) == "function")
end



local function test_method_call_without_self()
  print("\n=======> test_method_call_without_self")

  nt(1)
  _obj:ShowDialog()

  nt()
  _obj.ShowDialog(_obj)

  nt()
  _obj.ShowDialog({})

  nt()
  local res = pcall(_obj.ShowDialog, 1)
  assert(res)

  nt()
  res = pcall(_obj.ShowDialog)
  assert(res == false)
end

local function test_optional_params()
  nt(1)

  local teste = {}
  local obj = luacom.ImplInterfaceFromTypelib(teste, "test.tlb", "ITest1" )
  assert(obj)

  nt('[in, optional]')
  -- short test_optional(
  --   short p1, [optional] VARIANT p2, [optional] VARIANT p3)
  teste.test_optional = function(self, ...)
    --FIX: asserteq(tuple(...), tuple(3,4,5))
    return 6,7,8,9
  end
  -- print(tuple(obj:test_optional(3,4,5)))
  asserteq(tuple(obj:test_optional(3,4,5)), tuple(6))

  nt('[out, optional]')
  teste.test_out_optional = function(self, ...)
    asserteq(tuple(...), tuple(3))
    return 5,6,7,8
  end
  --print(tuple(obj:test_out_optional(3,4,5)))
  asserteq(tuple(obj:test_out_optional(3,4,5)), tuple(5,6,7))

  nt('[in, out, optional]')
  -- short test_in_out_optional(
  --   short p1, [in, out, optional] VARIANT* p2,
  --             [in, out, optional] VARIANT* p3)
  local par
  teste.test_in_out_optional = function(self, ...)
    asserteq(tuple(...), par)
    return 6,7,8,9
  end
  par = tuple(3,4,5)
  asserteq(tuple(obj:test_in_out_optional(3,4,5)), tuple(6,7,8))
  par = tuple(3,nil,nil)
  asserteq(tuple(obj:test_in_out_optional(3)), tuple(6,7,8))

end


local function test_in_params()
  print("\n=======> test_in_params")

--[[FIX?
  local obj = luacom.CreateObject(teste_progid)
  assert(obj)

  -- Testa ordem dos parametros

  local res = obj:TestParam1(1,2,3)
  assert(res == 1)

  local res = obj:TestParam2(1,2,3)
  assert(res == 2)

  local res = obj:TestParam3(1,2,3)
  assert(res == 3)
--]]
end



local function test_inout_params()
  print("\n=======> test_inout_params")

--[[DISABLED:
  local iupobj = iupolecontrol{progid = "TESTECTL.TesteCtrl.1"}
  assert(iupobj)

  local obj = iupobj.LUACOM

  nt(1)
  local res = pcall(obj.TestInOutshort, obj, 10)
  assert(res ~= nil)
  assert(res[1] == 20)

  nt()
  local res = pcall(obj.TestInOutfloat, obj,2.5)
  assert(res ~= nil)
  assert(res[1] == 5)

  nt()
  local res = pcall(obj.TestInOutlong, obj,100000)
  assert(res ~= nil)
  assert(res[1] == 200000)
--]]
end



local function test_out_params()
  print("\n=======> test_out_params")

--[[DISABLED:
  local iupobj = iupolecontrol{progid = "TESTECTL.TesteCtrl.1"}
  assert(iupobj)

  local obj = iupobj.LUACOM

  nt(1)
  local res = pcall(obj.TestOutParam, obj)
  assert(res ~= nil and res[1] == 1000)

  nt()
  local res = pcall(obj.Test2OutParams, obj)
  assert(res ~= nil)
  assert(res[1] == 1.1 and tonumber(res[2]) == res[1])

  nt()
  local res = pcall(obj.TestRetInOutParams, obj,2,3)
  assert(res ~= nil)
  assert(res[1] == 6)
  assert(res[2] == 2)
  assert(res[3] == 3)
--]]
end



-- teste de safe array
local function test_safearrays()
  print("\n=======> test_safearrays")

  local teste = {}

  nt(1)

  local obj = luacom.ImplInterfaceFromTypelib(teste, "test.tlb", "ITest2" )
  assert(obj)

  array = {}
  for i=1,3 do
    if array[i] == nil then
      array[i] = {}
    end
    for j=1,3 do
      if array[i][j] == nil then
        array[i][j] = {}
      end
      for k=1,3 do
        array[i][j][k] = i*9+j*3+k
      end
    end
  end

  teste.TestSafeArray = function(self, array_param)
    assert(table2string(array_param) == table2string(array))
  end

  for i=1,10 do
    obj:TestSafeArray(array)
  end

  nt()

  -- testa tratamento de erro na conversao

  teste.TestSafeArray2 = function(self, array_param)
    print(table2string(array_param))
    return 1
  end

  teste.TestSafeArrayVariant = function(self, array_param)
    return array_param
  end

  -- new in 1.4
  local a = obj:TestSafeArrayVariant({})
  assert(table2string(a) == table2string({}))

  -- note: arrays should not contain nil.
  local a = obj:TestSafeArrayVariant({nil,1})
  assert(table2string(a) == table2string({}))

  -- multi-dimensional arrays with zero sizes ambiguous?
  local res = pcall(obj.TestSafeArray2,obj,{{}})
  assert(res == false)

  local a = obj:TestSafeArrayVariant({1})
  assert(table2string(a) == table2string({1}))

  local a = obj:TestSafeArrayVariant({{1}})
  assert(table2string(a) == table2string({{1}}))

  local res = pcall(obj.TestSafeArray2,obj,{1,2,{3}})
  assert(res == false)

  local res = pcall(obj.TestSafeArray2,obj,{{1},{2},{3,4}})
  assert(res == false)

  local res = pcall(obj.TestSafeArray2,obj,{{{1,2}},{{2,1}},{{3}}})
  assert(res == false)

  -- steals a userdata...
  local userdata = obj["_USERDATA_REF_"]

  local res = pcall(obj.TestSafeArrayVariant,{obj,{userdata, userdata}})
  assert(res)

  nt()  -- tests long safe arrays

  local i = 0
  array = {}

  for i=1,100 do
    array[i] = i
  end

  a = obj:TestSafeArrayVariant(array)

  for i=1,100 do
    assert(a[i] == i)
  end

  local array_test = {{1},{"1"},{1},{1},{1},{1},{1},{1},{1},{1},{1},{1},{1}}
  a = obj:TestSafeArrayVariant(array_test)
  assert(table2string(a) == table2string(array_test))

  nt()

  teste.TestSafeArrayIDispatch = function(self, array)
    for i=1,9 do
      print(tostring(i).." => "..tostring(array[i].Teste))
    end
  end

  local obj = luacom.ImplInterfaceFromTypelib(teste, "test.tlb", "ITest2" )
  assert(obj)

  array = {}
  teste.Teste = 10

  for i=1,9 do
    array[i] = luacom.ImplInterfaceFromTypelib(teste, "test.tlb", "ITest2" )
  end

  obj:TestSafeArrayIDispatch(array)

  -- test: multidimensional row/column order (fixed in 1.4)
  local excel = luacom.CreateObject 'Excel.Application'
  if excel then
    local wb = excel.Workbooks:Add()
    local ws = wb.ActiveSheet
    local r = ws:Range("A2", "B5")
    r.Value2 = {{2,3},{4,5},{6,7},{8,9}}
    local v = r.Value2
    assert(table2string(v) == '{{2, 3}, {4, 5}, {6, 7}, {8, 9}}',
           table2string(v))
    assert(ws.Cells(2,2).Value2 == 3)
    assert(ws.Cells(3,1).Value2 == 4)
    wb.Saved = true
    wb:Close()
    excel = nil
  else
    print 'WARNING: test disabled (Excel not detected)'
  end

  collectgarbage()
end


-- Teste de luacom.NewObject
local function test_NewObject()
  print("\n=======> test_NewObject")

  nt(1)

  local tm_gettable = 
    function(table, index)
      rawset(table, "Accessed", 1)
      return rawget(table, index)
    end

  local tm_settable = 
    function(table, index, value)
      rawset(table, index, value)
    end

  local impl = {}
  local testtag = {}
  setmetatable(impl, testtag)
  testtag["__index"] =  tm_gettable
  testtag["__newindex"] = tm_settable

  local obj = luacom.NewObject(impl, "LUACOM.Test")
  assert(obj)

  obj.prop1 = 1
  assert(obj.prop1 == 1 and impl.Accessed == 1)
end


--
-- Teste da conversao de dados
--
local function test_DataTypes()
  print("\n=======> test_DataTypes")

  local teste = {}
  teste.Test = function(self, in_param, in_out_param, out_param)
    return in_out_param, in_param, in_param
  end

  local obj = luacom.ImplInterface(teste, "LUACOM.Test", "IDataConversionTest")
  assert(obj)

  nt(1)  -- Datas

  teste.TestDATE = teste.Test

  -- this one should work  
  local date = "29/2/1996 10:00:00"
  local date2 = "1/1/2001 01:00:00"
  local date_res1, date_res2, date_res3 = obj:TestDATE(date, date2)
  assert(date_res1:find '01')
  assert(date_res2:find '96')
--print(date_res3, date_res2)
--FIX  assert(date_res3 == date_res2)

  -- this ones should fail
  local date = "29/2/1997 10:00:00"
  local res = pcall(obj.TestDATE,obj, date, date)
  assert(res == false)

  local date = "1/1/2001 24:00:00"
  local res = pcall(obj.TestDATE,obj, date, date)
  assert(res == false)

  nt("Currency") -- teste de currency

  teste.TestCURRENCY = teste.Test
  local cy = "R$ 1.000.001,15"
  -- cy = "$ 1,000,001.15" -- or this
  local cy2 = 988670.12
  print 'FIX - remove regional settings dependency in test'; skipped=true
  -- cy_res2, cy_res, cy_res3 = obj:TestCURRENCY(cy, cy2)
  -- assert(cy_res == 1000001.15)
  -- assert(cy_res2 == cy2)
  -- assert(cy_res3 == 1000001.15)
  local cy = 12345.56
  local cy_res = obj:TestCURRENCY(cy, cy)
  assert(cy_res == cy)

  -- Este deve falhar

  local cy = "R$ 1,000,001.15"
  local res = pcall(obj.TestCURRENCY,obj, cy)
  assert(res == false)

  local cy = {}
  local res = pcall(obj.TestCURRENCY,obj, cy)
  assert(res == false)

  nt() -- teste de booleanos

  teste.TestBool = teste.Test

  local b = true
  local b2 = false
  local b_res2, b_res, b_res3 = obj:TestBool(b, b2)
  assert(b == b_res)
  assert(b2 == b_res2)
  assert(b_res3 == b)

  local b = false
  local b_res = obj:TestBool(b, b)
  assert(b == b_res)

  nt() -- teste de unsigned char

  teste.TestChar = teste.Test

  local uc = 100
  local uc2 = 150
  local uc_res2, uc_res, uc_res3 = obj:TestChar(uc, uc2)
  assert(uc_res == uc)
  assert(uc_res2 == uc2)
  assert(uc_res3 == uc)

  -- este deve falhar
  local uc = 1000
  local res = pcall(obj.TestChar, obj, uc, uc)
  assert(res == false)

  -- teste de VARIANT
  teste.TestVARIANT = teste.Test

  nt("VARIANT - number")
  local v = 100
  local v2 = 150
  local v_res2, v_res, v_res3 = obj:TestVARIANT(v, v2)
  assert(v_res == v)
  assert(v_res2 == v2)
  assert(v_res3 == v)

  nt("VARIANT - String")
  local v = "abcdef"
  local v2 = "ghijog"
  local v_res2, v_res, v_res3 = obj:TestVARIANT(v, v2)
  assert(v_res == v)
  assert(v_res2 == v2)
  assert(v_res3 == v)

  nt("VARIANT - mixed")
  local v = "abcdef"
  local v2 = 123
  local v_res2, v_res, v_res3 = obj:TestVARIANT(v, v2)
  assert(v_res == v)
  assert(v_res2 == v2)
  assert(v_res3 == v)

  nt("VARIANT - IDispatch")
  local v = obj
  local v2 = obj
  local v_res2, v_res, v_res3 = obj:TestVARIANT(v, v2)
print 'FIX: GetIUnknown test disabled'; skipped=true
--  assert(luacom.GetIUnknown(v_res)== luacom.GetIUnknown(v))
--  assert(luacom.GetIUnknown(v_res2)== luacom.GetIUnknown(v2))
--  assert(luacom.GetIUnknown(v_res3)== luacom.GetIUnknown(v))

  nt("VARIANT - IUnknown")
--[[FIX  v = luacom.GetIUnknown(obj)
  local v2 = v
  local v_res2, v_res, v_res3 = obj:TestVARIANT(v, v2)
  assert(luacom.GetIUnknown(v_res) == v)
  assert(luacom.GetIUnknown(v_res2) == v2)
  assert(luacom.GetIUnknown(v_res3) == v)
]]

  nt("Enum") -- Teste de Enum
  teste.TestENUM = teste.Test
  local e = 3
  local e2 = 4
  local e_res2, e_res, e_res3 = obj:TestENUM(e, e2)
  assert(e_res == e)
  assert(e_res2 == e2)
  assert(e_res3 == e)

  nt("BSTR") -- Teste de BSTR
  teste.TestBSTR = teste.Test
  local s = "abdja"
  local s2 = "fsjakfhdksjfhkdsa"
  local s_res2, s_res, s_res3 = obj:TestBSTR(s, s2)
  assert(s_res == s)
  assert(s_res2 == s2)
  assert(s_res3 == s)

  -- embedded nulls
  --   bug#428 - "Improper handling of BSTRs with embedded zeros"
  local s = "asdf\0asdf"; s2 = s
  local s_res2, s_res, s_res3 = obj:TestBSTR(s, s2)
  assert(s_res == s and s_res2 == s and s_res3 == s)

  -- empty string
  local s = ""; s2 = s
  local s_res2, s_res, s_res3 = obj:TestBSTR(s, s2)
  assert(s_res == s and s_res2 == s and s_res3 == s)

  nt("Short")
  teste.TestShort = teste.Test
  local p1 = 10
  local p2 = 100
  local res2, res1, res3 = obj:TestShort(p1, p2)
  assert(p1 == res1)
  assert(p2 == res2)
  assert(p1 == res3)

  nt("Long")
  teste.TestLong = teste.Test
  local p1 = 1234567
  local p2 = 9876543
  local res2, res1, res3 = obj:TestLong(p1, p2)
  assert(p1 == res1)
  assert(p2 == res2)
  assert(p1 == res3)

  nt("Float")
  teste.TestFloat = teste.Test
  local p1 = 10.111
  local p2 = 123.8212
  local res2, res1, res3 = obj:TestFloat(p1, p2)
  assert(math.abs(p1-res1) < 0.00001)
  assert(math.abs(p2-res2) < 0.00001)
  assert(math.abs(p1-res3) < 0.00001)

  nt("Double")
  teste.TestDouble = teste.Test
  local p1 = 9876.78273823
  local p2 = 728787.889978
  local res2, res1, res3 = obj:TestDouble(p1, p2)
  assert(p1 == res1)
  assert(p2 == res2)
  assert(p1 == res3)

  nt("Int")
  teste.TestInt = teste.Test
  local p1 = 1000
  local p2 = 1001
  local res2, res1, res3 = obj:TestInt(p1, p2)
  assert(p1 == res1)
  assert(p2 == res2)
  assert(p1 == res3)

  nt("IDispatch")
  teste.TestIDispatch = teste.Test
  local p1 = obj
  local p2 = obj
  local res2, res1, res3 = obj:TestIDispatch(p1, p2)
--[[FIX
  assert(luacom.GetIUnknown(res1) == luacom.GetIUnknown(obj))
  assert(luacom.GetIUnknown(res2) == luacom.GetIUnknown(obj))
  assert(luacom.GetIUnknown(res3) == luacom.GetIUnknown(obj))
]]

  nt("IUnknown")
  teste.TestIUnknown = teste.Test
  local p1 = luacom.GetIUnknown(obj)
  local p2 = luacom.GetIUnknown(obj)
  local res2, res1, res3 = obj:TestIUnknown(p1, p2)
--[[FIX
  assert(p1 == luacom.GetIUnknown(res1))
  assert(p2 == luacom.GetIUnknown(res2))
  assert(p1 == luacom.GetIUnknown(res3))
]]
end


local function test_USERDEF_PTR()
  print("\n=======> test_USERDEF_PTR")

  local t = {}
  function t.__index(table, index)
    return rawget(table, "teste")
  end

  local teste_table = setmetatable({}, t)
  -- por default simplesmente retorna
  teste_table.teste = function(self, value)
    return value
  end

  local teste  = luacom.ImplInterface(teste_table, "LUACOM.Test", "ITest1")
  local teste2 = luacom.ImplInterface(teste_table, "LUACOM.Test", "IStruct1")
  assert(teste)
  assert(teste2)

  -- outra opcao
  local o2
  local teste_disp = function(self, value)
    local t = {}
    o2 = luacom.ImplInterface(t, "LUACOM.Test", "ITest1")
    return o2
  end

  nt(1)
  local n = teste:up_userdef_enum(3)
  assert(n == nil)

  nt()
  local n = teste:up_ptr_userdef_enum(4)
--FIX  assert(n == 4)

  nt()
  teste_table.up_ptr_disp = teste_disp

  local n = teste:up_ptr_disp(teste2)
print 'FIX: GetIUnknown test disabled'; skipped=true
--  assert(luacom.GetIUnknown(n) == luacom.GetIUnknown(o2))

  nt()
  local n = teste:up_ptr_userdef_disp(teste2)
  assert(n == nil)

  nt()
  teste_table.up_ptr_ptr_userdef_disp = teste_disp
  local n = teste:up_ptr_ptr_userdef_disp(teste2)
print 'FIX: GetIUnknown test disabled'; skipped=true
--  assert(luacom.GetIUnknown(n) == luacom.GetIUnknown(o2))

  nt()
  local n = teste:up_userdef_alias(1)
  assert(n == nil)

  nt()
  local n = teste:up_ptr_userdef_alias(1)
--FIX  assert(n == 1)
  local n = teste:up_userdef_alias_userdef_alias(1)
  assert(n == nil)

  nt()
  local n = teste:up_userdef_disp(teste2)
  assert(n == nil)

  nt()
  teste_table.up_udef_alias_ptr_udef_alias_ptr_disp = teste_disp
  local n = teste:up_udef_alias_ptr_udef_alias_ptr_disp(teste2)
print 'FIX: GetIUnknown test disabled\n'; skipped=true
--  assert(luacom.GetIUnknown(n) == luacom.GetIUnknown(o2))

  nt()
  teste_table.up_userdef_disp = function(self, value)
    o2 = value
  end
  teste:up_userdef_disp(teste2)
--  assert(luacom.GetIUnknown(teste2) == luacom.GetIUnknown(o2))

  nt("Unknown & Userdef")

  local teste_unk = function(self, value)
    local t = {}
    o2 = luacom.ImplInterface(t, "LUACOM.Test", "ITest1")
    o2 = luacom.GetIUnknown(o2)
    return o2
  end

  teste_table.up_udef_alias_ptr_udef_alias_ptr_unk = teste_unk

  local n
    = teste:up_udef_alias_ptr_udef_alias_ptr_unk(luacom.GetIUnknown(teste2))
  --assert(luacom.GetIUnknown(n) == o2)

  nt("Unknown & Userdef")
  teste_table.up_userdef_unk = function(self, value)
    o2 = value
  end

  teste:up_userdef_unk(luacom.GetIUnknown(teste2))
--  assert(luacom.GetIUnknown(teste2) == luacom.GetIUnknown(o2))
end


local function test_field_redefinition()
  print("\n=======> test_field_redefinition")  

  nt(1)
  local luaobj = {}
  luaobj.getput = 1
  luaobj.get = 2
  luaobj.func = function() return 3 end

  local obj = luacom.ImplInterface(luaobj, "LUACOM.Test", "ITest1")
  assert(obj)
  assert(obj.getput == 1)
  assert(obj.get == 2)
  assert(obj:func() == 3)

  nt()
  obj.getput = function() return 2 end
  assert(obj:getgetput() == 2)
  local settable = function(t, i,v )
    t[i] = v
    return 1
  end

  -- these ones should fail
  nt()
  local res = pcall(settable, obj, "get", 1)
  assert(res == false)

  local res = pcall(settable,obj, "func", 1)
  assert(res == false)

  nt()
  local d, res = pcall(settable,obj, "get", print)
  assert(res == 1)

  local d, res = pcall(settable,obj, "func", print)
  assert(res == 1)
end


local function test_indexing()
  print("\n=======> test_indexing: NOT IMPLEMENTED!!!")  
end

local function test_API()
  nt(1)
  
  nt("CreateLuaCOM")

  local punk = luacom.GetIUnknown(_obj)
  assert(punk)
  
  local obj 
 
  local obj = luacom.CreateLuaCOM(1)
  assert(obj == nil)
  
  local obj = luacom.CreateLuaCOM(punk)
  assert(type(obj.prop1) == "number")
  
  nt("ImportIUnknown & .IUnknown")
  
  local ptr = _obj.IUnknown
  assert(type(ptr) == "userdata")
  
  local obj = luacom.ImportIUnknown(ptr)
  assert(obj)
  
  local obj = luacom.CreateLuaCOM(obj)
  assert(type(obj.prop1) == "number")
end


----------------------------------------------------------------------
----------------------------------------------------------------------


-- Que testes vao ser realizados
init()

test_simple()
test_API()
test_wrong_parameters()
test_propget()
test_propput()
test_index_fb()
test_method_call_without_self()

test_in_params()
test_out_params()
test_inout_params()
test_optional_params()

test_iface_implementation()
test_connection_points()

test_safearrays()

if false then
test_stress()
end

test_NewObject()
test_DataTypes()
test_USERDEF_PTR()

test_field_redefinition()
test_indexing()

if skipped then
  print 'WARNING: some tests were disabled.\n'
end

finish()

return
