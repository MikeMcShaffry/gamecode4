LOCKS = 8

if not _sleep then
_sleep = function(n)
	for i=1, 100000*n do end
 end
end

m = thread.newmutex()

function child()
    print("child attempting lock.")
    m:lock()
    print("child got lock.")
end

m:lock()
print("parent got lock.")

thread.newthread(child, {})

for i = 1, LOCKS do
    m:lock()
    print("parent re-locked.")
    _sleep(1)
end

for i = 1, LOCKS do
    _sleep(1)
    print("parent unlocking.")
    m:unlock()
end

_sleep(1)

print("parent unlocking last.")
m:unlock()

_sleep(1)

print("parent gone.")
