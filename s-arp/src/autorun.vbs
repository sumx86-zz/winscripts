Option Explicit

dim hkey: hkey = "HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Run\"
dim bdir: bdir = "\build\Debug\"
dim exe: exe = "s-arp.exe"
dim wsh, fso
set wsh = CreateObject("WScript.Shell")
set fso = CreateObject("Scripting.FileSystemObject")

wsh.RegWrite hkey & exe, fso.GetParentFolderName(WScript.ScriptFullName) & bdir & exe, "REG_SZ"

set wsh = Nothing
set fso = Nothing