-- Cookie data will be shared across the easy handles to do an authorized download
require("cURL")

-- create share handle (share COOKIE and DNS Cache)
s = cURL.share_init()
s:setopt_share("COOKIE")
s:setopt_share("DNS")

-- create first easy handle to do the login
c = cURL.easy_init()
c:setopt_share(s)
c:setopt_url("http://targethost/login.php?username=foo&password=bar")

-- create second easy handle to do the download
c2 = cURL.easy_init()
c2:setopt_share(s)
c2:setopt_url("http://targethost/download.php?id=test")

--login
c:perform()

--download			
c2:perform()			


