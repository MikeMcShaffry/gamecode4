-- try eBay login
require("cURL")

headers = {"Accept: text/*",
	   "Accept-Language: en",
	   "Accept-Charset: iso-8859-1,*,utf-8",
	   "Cache-Control: no-cache"}
login1_url = "https://signin.ebay.com/ws/eBayISAPI.dll?SignIn"
login2_url = "https://signin.ebay.com/ws/eBayISAPI.dll?SignInWelcome&userid=%s&pass=%s&keepMeSignInOption=1"

c = cURL.easy_init()
c:setopt_useragent("Mozilla/5.0 (Windows; U; Windows NT 5.0; de; rv:1.8.1.6) Gecko/20070725 Firefox/2.0.0.6")
c:setopt_httpheader(headers)
c:setopt_cookiefile("")
c:setopt_followlocation(1)


c:setopt_url(login1_url)
-- ignore output, just set cookie
c:perform({writefunction = function (str) end})

print("Username:")
username=io.read()
print("Password:")
password=io.read()

c:setopt_url(string.format(login2_url, c:escape(username), c:escape(password)))
c:perform({writefunction=function(str) 
			    succeed = succeed or (string.find(str, "srcId:%s+SignInAlertSupressor--")) 
			 end })

if (succeed) then
   print("Login succeed")
else
   print("Login failed")
end
   
