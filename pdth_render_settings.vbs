'Set the refresh rate for Payday: The Heist
Option Explicit

const ForReading = 1, ForWriting = 2
dim wsh: set wsh = CreateObject("WScript.Shell")
dim fso: set fso = CreateObject("Scripting.FileSystemObject")
dim file: file = "C:\Users\" & wsh.ExpandEnvironmentStrings("%USERNAME%") & "\Appdata\Local\PAYDAY\renderer_settings.xml"
dim pattern: pattern = "refresh_rate(\s|)=(\s|)(""\d{2,3}"")"

if fso.FileExists(file) then
    dim lines: lines = ReadFile(file)
    dim line, matches
    dim content
    dim re: set re = new RegExp
    re.Pattern = pattern
    for each line in lines
        set matches = re.Execute(line)
        if matches.Count = 1 then                              'change this value here
            line = Replace(line, matches.Item(0).Submatches(2), """" & 144 & """")
        end if
        content = content & line & vbCrlf
    next
    WriteToFile file, content
    MsgBox "New refresh rate set successfully!"
end if

function ReadFile(ByVal file)
    dim ofs: set ofs = fso.OpenTextFile(file, ForReading)
    dim lines: lines = Array()

    do while not ofs.AtEndOfStream
        dim line: line = ofs.ReadLine()
        if len(line) > 0 then
            AddLine line, lines
        end if
    loop
    ofs.Close()
    set ofs = Nothing: ReadFile = lines
end function

function WriteToFile(ByVal file, ByVal content)
    dim ofs: set ofs = fso.OpenTextFile(file, ForWriting)
    ofs.Write content
    ofs.Close()
    set ofs = Nothing
end function

function AddLine(ByVal line, ByRef arr)
    Redim Preserve arr(Ubound(arr) + 1)
    arr(UBound(arr)) = line
end function

set wsh = Nothing
set fso = Nothing