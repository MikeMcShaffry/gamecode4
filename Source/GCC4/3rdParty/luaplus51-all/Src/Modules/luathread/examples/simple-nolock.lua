function flood(word)
    while 1 do io.write(word, "\n") end
end

thread.newthread(flood, {"child"})
flood("parent")
