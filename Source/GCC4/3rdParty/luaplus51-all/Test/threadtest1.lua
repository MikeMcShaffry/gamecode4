local function flood(output, word)
    while 1 do 
        output:lock()
        io.write(word, ", ")
        output:unlock()
    end
end

local output = thread.newmutex()
thread.newthread(flood, {output, "child"})
flood(output, "parent")

