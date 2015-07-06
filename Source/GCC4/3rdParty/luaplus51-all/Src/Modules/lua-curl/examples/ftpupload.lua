-- simple "On the fly" fileupload

require("cURL")

c=cURL.easy_init()
c:setopt_url("ftp://ftptest:secret0815@targethost/file.dat")
c:setopt_upload(1)
count=0
c:perform({readfunction=function(n) 
			   count = count + 1
			   if (count < 10)  then
			      return "Line " .. count .. "\n"
			   end
			   return nil
			end})
print("Fileupload done")
