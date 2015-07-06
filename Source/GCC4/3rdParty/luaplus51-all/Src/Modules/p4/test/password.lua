require 'p4'
p4 = P4.P4()
p4.password = "myoldpass"

pcall(function()
    p4:connect()
    p4:run_password( "myoldpass", "mynewpass" )
end)

for i, e in ipairs(p4.errors) do
    print(e)
end

p4:disconnect()
