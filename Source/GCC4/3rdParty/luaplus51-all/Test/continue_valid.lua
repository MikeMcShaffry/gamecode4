--[[ Repeat loops ]]

-- Simple repeat
repeat
  print "Good 1"
until true

-- Simple repeat with continue
repeat
  continue
until true
print "Good 2"

-- Repeat with code after a continue
repeat
  print "Good 3"
  if true then
    continue
  end
  print "Bad 1"
until true

-- Repeat with locals before and after a continue and code after a deep continue
do
  local n = 0
  local set = {}
  
  repeat
    local p = n
    n = n + 1
    if p == 1 then
      if p then
        continue
      end
    end
    local q = true
    set[p] = q
  until p == 2
  
  if set[0] and set[2] and not set[1] and not set[3] then
    print "Good 4"
  else
    print "Bad 2"
  end
end

-- Repeat with upvalues and continue
do
  local n = 0
  local set = {}
  
  repeat
    local p = n
    n = n + 1
    set[p] = function() p = p + 1; return p end
    if p == 1 then
      continue
    end
    p = p - 1
  until p == 2
  
  if set[0]() == 0 and set[1]() == 2 and set[2]() == 2 then
    if set[0]() == 1 and set[2]() == 3 then
      print "Good 5"
    else
      print "Bad 3"
    end
  else
    print "Bad 4"
  end
end

-- Even complex until clauses are executed after a continue
do
  local good = false
  repeat
    local p = true
    if p then
      continue
    end
    print "Bad 5"
  until (function() good = function() return p end; return p end)()
  if good then
    print "Good 6"
  else
    print "Bad 6"
  end
end

--[[ Numeric for loops ]]

-- Continue on non-final iteration
do
  local set = {}
  for i = 1, 3 do
    if i == 2 then
      continue
    end
    set[i] = i
  end
  if set[1] and set[3] and not set[2] and not set[4] then
    print "Good 7"
  else
    print "Bad 7"
  end
end

-- Continue on all iterations
do
  local set = {}
  for i = 1, 3 do
    set[i] = i
    continue
  end
  if set[1] and set[2] and set[3] and not set[4] then
    print "Good 8"
  else
    print "Bad 8"
  end
end

--[[ Generic for loops ]]

-- Continue on single iteration
do
  local input = {a = true, b = true, c = false}
  local output = {}
  for k, v in pairs(input) do
    if not v then
      continue
    end
    output[k] = v
  end
  if output.a and output.b and not output.c then
    print "Good 9"
  else
    print "Bad 9"
  end
end

-- Continue on every iteration
do
  local values = {false, false, false}
  for k in ipairs(values) do
    values[k] = true
    if values[k] then
      continue
    end
  end
  if values[1] and values[2] and values[3] then
    print "Good 10"
  else
    print "Bad 10"
  end
end

--[[ While loops ]]

-- Continue on one iteration, break on one iteration
do
  local i = 0
  local set = {}
  while true do
    i = i + 1
    if i == 2 then
      continue
    elseif i == 4 then
      break
    end
    set[i] = true
  end
  if set[1] and not set[2] and set[3] and not set[4] then
    print "Good 11"
  else
    print "Bad 11"
  end
end

-- Continue on every iteration
do
  local i = 1
  local set = {}
  while i <= 3 do
    set[i] = true
    i = i + 1
    do
      continue
    end
    set = {}
  end
  if set[1] and set[2] and set[3] and not set[4] then
    print "Good 12"
  else
    print "Bad 12"
  end
end
  