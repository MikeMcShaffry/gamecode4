--
-- `continue' statement test
--

i = 0
while i < 7 do
    i = i + 1
    if i == 3 then continue end
    if i == 6 then break end
    io.write(i)
end

io.write(" ")

i = 0
repeat
    i = i + 1
    if i == 3 then continue end
    io.write(i)
until i == 5

io.write(" ")

for i = 1, 5 do
    if i == 3 then continue end
    io.write(i)
end

print()

print(loadstring("if 2 > 1 then continue end"))
print(loadstring("do continue end"))

--
-- the result is:
--
-- 1245 1245 1245
-- nil	[string "if 2 > 1 then continue end"]:1: no loop to continue near `end'
-- nil	[string "do continue end"]:1: no loop to continue near `end'

