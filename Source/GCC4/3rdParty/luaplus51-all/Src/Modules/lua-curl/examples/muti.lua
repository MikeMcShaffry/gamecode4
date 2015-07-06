require("cURL")

-- setup easy 
c = cURL.easy_init()
c2 = cURL.easy_init()
-- setup url
c:setopt_url("http://www.example.com/")
c2:setopt_url("http://www.hoetzel.info/")
-- c.perform()
m = cURL.multi_init()
m2 = cURL.multi_init()
m:add_handle(c)
m2:add_handle(c2)
-- perform, 
-- it = m:perform()

for data, type, easy in m:perform() do 
   if (type == "data" and c == easy) then print(data) end
end

-- for data,type,easy in m2:perform() do 
--    if (type == "header") then print(data) end
-- end