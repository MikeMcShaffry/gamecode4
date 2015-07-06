-- test md5 library

if md5 then
 KNOWN="df30c0941d1f8eef9a5df9e482f0a3da"
 COMMAND="echo -n 'md5sum\t'; md5sum "
else
 md5=sha1
 KNOWN="562abb5ed8f9d5ab3e12387108aa36ca2c61b7e9"
 COMMAND="echo -n 'sha1sum\t'; sha1sum "
end

print(md5.version)
print""

function report(w,s,F)
 print(w,s.."  "..F)
 assert(s==KNOWN)
end

F="README"

assert(io.input(F))
report("all",md5.digest(io.read"*a"),F)

assert(io.input(F))
d=md5.new()
while true do
 local c=io.read(1)
 if c==nil then break end
 d:update(c)
end
report("loop",d:digest(),F)
report("again",d:digest(),F)

assert(io.input(F))
d:reset()
while true do
 local c=io.read(math.random(1,16))
 if c==nil then break end
 d:update(c)
end
report("reset",d:digest(),F)

os.execute(COMMAND..F)
report("known",KNOWN,F)

print""
print(md5.version)

-- eof
