----------------------------
-- Authentication configuration file
-- 
-- Defines the method and details of the authentication mechanism
--
-- $Id: authentication_conf.lua,v 1.1 2007/12/05 18:40:17 carregal Exp $
----------------------------

require"cgilua.authentication"

local options = {
    -- Authentication method: "simpledatabase", "webserver", "ldap", "test"
    method = "test",
    
    -- How Authentication is stored on the client
    -- This directive can be "cookie" or "url"
    tokenPersistence="url",
    
    -- Name used for the token persitence
    tokenName = "userhash",
    
    -- The criptografic key to encode user information on cookies and URLs
    criptKey="sfdtdyedbbg",
    
    -- Where to redirect when authentication is needed
    -- checkURL="https://remotehost/sso/authenticate", -- absolute URL
    checkURL="check.lua", -- authentication controller used by app.lua
}

--
-- Simple database authentication parameters
--
options.simpledatabase = {
    driver="mysql",  -- "mysql","oracle","odbc"
    sourcename="users",
    dbusername="root",
    dbpassword="pass",
    passwd_hash_function=(require"md5") and md5.sumhexa, -- for MD5 encription
        -- passwd_hash_function = function(arg) return arg end , -- for no encription
    users_table="Users",
    user_name_field="Name",
    pass_field="Password",
}

--
-- Web Server authentication parameters
--
options.webserver = {
    user_env_var="REMOTE_USER", -- if supports RFC 931 use "REMOTE_IDENT"
}

--
-- LDAP authentication parameters
--
options.ldap = {
    host="127.0.0.1", -- accepts host:port
    usetls=false,
    --baseDN="ou=People,dc=example,dc=com",
    baseDN="dc=example,dc=com",
    RDNatributeName="cn",
}


local methods = {
    simpledatabase = {},
    webserver = {},
    ldap = {},
    test = {},
}

--
-- Simple database authentication method
--
methods.simpledatabase.check = function(username,userpasswd)
    local simpledatabase = options.simpledatabase
    require ("luasql."..simpledatabase.driver)

    local query = "SELECT "..simpledatabase.pass_field.." FROM "..simpledatabase.users_table.." WHERE "..simpledatabase.user_name_field.." = '"..username.."'"
    local conn,err = luasql[simpledatabase.driver]:connect(simpledatabase.sourcename,simpledatabase.dbusername,simpledatabase.dbpassword)
    if conn == nil then
        return false, err
    end
    local cur=conn:execute(query)
    local row=cur:fetch()
    local retpasswd=row[simpledatabase.pass_field]
    local passwdhash=simpledatabase.passwd_hash_function(userpasswd)
    if retpasswd==passwdhash then
        return true
    end
    return false, "Username and password do not match."
end


--
-- Web Server authentication method
--
methods.webserver.user = function ()
	return cgilua.servervariable (options.webserver.user_env_var)
end


--
-- LDAP authentication method
--
methods.ldap.check = function(username,userpasswd)
    require"lualdap"
	local ld,err = options.lualdap.open_simple (options.ldap.host,
                options.ldap.RDNatributeName.."="..username..","..options.ldap.baseDN,
		        userpasswd, options.ldap.usetls)
	if ld~=nil then
		return true
	else
	    return false, err
    end
end

--
-- Test authentication method
--
methods.test.check = function(username,userpasswd)
    if (username == "admin") and (userpasswd == "admin") then
        return true
    else
        return false, "Wrong user/password combination!"
    end
end

cgilua.authentication.configure(options, methods)