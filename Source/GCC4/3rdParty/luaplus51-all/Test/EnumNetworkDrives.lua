require "luacom"

WshNetwork = luacom.CreateObject("WScript.Network");
oDrives = WshNetwork:EnumNetworkDrives();
print("Network drive mappings:");
--LuaDumpFile("c:\\test.txt", "oDrives", oDrives, true)
len = oDrives.length
print(len)
for i = 0, len - 1, 2 do
	print ("Drive " .. oDrives:Item(i) .. " = " .. oDrives:Item(i + 1));
end
