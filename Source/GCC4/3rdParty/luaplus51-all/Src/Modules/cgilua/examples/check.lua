-- Checking script example
-- Assumes that the login form will use two fields called username and pass

local username = cgilua.POST.username
local pass = cgilua.POST.pass
local logged, err, logoutURL

if cgilua.authentication then
    logged, err = cgilua.authentication.check(username, pass)
    username = cgilua.authentication.username() or ""
    logoutURL = cgilua.authentication.logoutURL()
else
    logged = false
    err = "No authentication configured!"
    username = ""
end

if logged and username then
	cgilua.redirect(cgilua.authentication.refURL())
else
    err = err or ""

	cgilua.htmlheader()
	cgilua.lp.include ("login.lp", {
        logged = logged, errorMsg = err, username = username,
        cgilua = cgilua, logoutURL = logoutURL})
end

