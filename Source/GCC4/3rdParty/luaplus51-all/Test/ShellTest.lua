require "com"

WshShell = com.CreateObject("WScript.Shell");
--strDesktop = WshShell.SpecialFolders("Desktop")
strDesktop = WshShell.SpecialFolders:Item("Desktop");
oShellLink = WshShell:CreateShortcut(strDesktop .. "\\Shortcut Script.lnk");
oShellLink.TargetPath = "c:\\test.txt";
oShellLink.WindowStyle = 1;
oShellLink.Hotkey = "CTRL+SHIFT+F";
oShellLink.IconLocation = "notepad.exe, 0";
oShellLink.Description = "Shortcut Script";
oShellLink.WorkingDirectory = strDesktop;
oShellLink:Save();
