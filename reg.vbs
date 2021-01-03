'Remove Activate Windows watermark
dim hkey : hkey = "HKEY_CURRENT_USER\Control Panel\Desktop\PaintDesktopVersion"
dim wsh
set wsh = CreateObject("WScript.Shell")
wsh.RegWrite hkey, 0, "REG_DWORD"