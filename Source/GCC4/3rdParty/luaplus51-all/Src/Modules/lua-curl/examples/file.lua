require("cURL")

-- open output file
f = io.open("example_homepage", "w")

c = cURL.easy_init()
-- setup url
c:setopt_url("http://www.example.com/")
-- perform, invokes callbacks
c:perform({writefunction = function(str) 
				f:write(str)
			     end})

-- close output file
f:close()
print("Done")


