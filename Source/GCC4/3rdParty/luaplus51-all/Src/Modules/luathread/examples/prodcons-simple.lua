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

function consumer(fifo, output)
	while 1 do
        local value = fifo:remove()
        output:lock()
		io.write("consumer removed ", value, "\n")
        output:unlock()
	end
end

function producer(count, fifo, output)
	while 1 do
        local value = count()
        fifo:insert(value)
        output:lock()
        io.write("producer inserted ", value, "\n")
        output:unlock()
	end
end

thread.newthread(consumer, {fifo, output})

producer(count, fifo, output)
