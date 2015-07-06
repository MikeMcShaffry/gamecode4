PRODUCERS = 9
CONSUMERS = 10
SIZE = 128

local thread = require("thread")
local queue = require("thread.queue")
local fifo = queue.newqueue(SIZE)
local output = thread.newmutex()

function newcount()
    local mutex = thread.newmutex()
    local value = 1
    return function ()
        mutex:lock()
        local v = value
        value = value + 1
        mutex:unlock()
        return v
    end
end

local count = newcount()

function consumer(id, fifo, output)
	while 1 do
        local value = fifo:remove()
        output:lock()
		io.write("[", id, "] got ", value, "\n")
        output:unlock()
	end
end

function producer(id, count, fifo, output)
	while 1 do
        local value = count()
        fifo:insert(value)
        output:lock()
        io.write("(", id, ") inserted ", value, "\n")
        output:unlock()
	end
end

for id = 1, CONSUMERS do
    thread.newthread(consumer, {id, fifo, output})
end

for id = 1, PRODUCERS-1 do
    thread.newthread(producer, {id, count, fifo, output})
end
producer(PRODUCERS, count, fifo, output)
