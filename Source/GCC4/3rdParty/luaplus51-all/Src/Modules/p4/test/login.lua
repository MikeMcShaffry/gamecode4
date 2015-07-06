require 'p4'
p4 = P4.P4()
p4.user = "Joshua"
p4.password = "my_password"
p4:connect()
p4:run_login()
opened = p4:run_opened()

