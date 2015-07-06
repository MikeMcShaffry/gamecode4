require 'p4'
p4 = P4.P4()
p4.user = "MyUser"
p4.password = "MyOldPassword"
p4:connect()
p4:run_password("MyOldPassword", "MyNewPassword")
-- p4.password is automatically updated with the encoded password
