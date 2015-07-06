require 'p4'
p4 = P4.P4()
p4:connect()
files = p4:run_filelog('index.html')
print(files[1])
for _, r in ipairs(files[1].revisions) do
    print(r)
    for _, i in ipairs(r.integrations) do
        print(i)
	end
end
