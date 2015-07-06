function flood(word)
    while 1 do 
        output:lock()
        io.write(word, "\n")
        output:unlock()
    end
end

output = thread.newmutex()
thread.newthread(flood, {"child"})
flood("parent")
