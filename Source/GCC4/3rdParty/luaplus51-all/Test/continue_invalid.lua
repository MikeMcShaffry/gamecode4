local good = true
function expect_error(name, code, no_err)
  local chunk, err = loadstring(code, "code")
  if not err == not not no_err then
    print(name .. ": Pass (" .. (err or "") .. ")")
  else
    print(name .. ": FAIL (" .. (err or "") .. ")")
    good = false
  end
end

function expect_noerror(name, code)
  return expect_error(name, code, true)
end

-- If continue isn't a language construct, then this will fail (and everything else will pass)
expect_noerror("Simple continue usage", [[
  while true do
    continue
  end
]])

expect_error("Invalid local usage in until clause", [[
  repeat
    local Q
    if Q then
      continue
    end
    local p = true
    Q = not Q
  until p
]])

expect_error("Invalid local usage as upvalue in until clause", [[
  repeat
    if Q then
      continue
    end
    local p = true
    Q = not Q
  until F(function() return p end)
]])

expect_error("Continue must be last statement within a block", [[
  repeat
    continue
    x = true
  until Q
]])

expect_error("Invalid usage of actvar slot which was valid at the time of the continue", [[
repeat
  do
    local p = 1
    continue
  end
  local q = 2
until q == 1
]])

expect_error("Multiple continue usage", [[
  for i = 1, 10 do print(i) end -- Increment PC and line count a bit
  repeat
    if false then continue end
    local AA
    if false then continue end
  until AA
]])

expect_error("Continue outside of a loop", [[
  continue
]])

if not good then
  print "At least one failure!"
end
