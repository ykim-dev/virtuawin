; Setup script for VirtuaWin using Inno Setup

[Setup]
AppName=VirtuaWin
AppVerName=VirtuaWin v3.1 test 1
DefaultDirName={pf}\VirtuaWin
DefaultGroupName=VirtuaWin
MinVersion=4,4
Uninstallable=1
OutputDir=.\output
BackColor=clLime
SolidCompression=yes
Compression=bzip/9
LicenseFile=COPYING.TXT

[Languages]
Name: "en"; MessagesFile: "VirtuaWin5.0.ISL"

[Tasks]
Name: autostart; Description: "Autostart upon boot";

[Dirs] 
Name: "{app}\icons"; Flags: uninsalwaysuninstall 
Name: "{app}\modules"; Flags: uninsalwaysuninstall 

[Files]
Source: "VirtuaWin.exe";  DestDir: "{app}"
Source: "VirtuaWin.hlp";  DestDir: "{app}"
Source: "WinList.exe";    DestDir: "{app}\Modules\"
Source: "VWAssigner.exe"; DestDir: "{app}\Modules\"
Source: "UserList.cfg";   DestDir: "{app}"; Flags: onlyifdoesntexist;
Source: "Tricky.cfg";     DestDir: "{app}"; Flags: onlyifdoesntexist;
Source: "COPYING.TXT";    DestDir: "{app}"
Source: "HISTORY.TXT";    DestDir: "{app}"
Source: "README.TXT";     DestDir: "{app}"; Flags: isreadme

[Icons]
Name: "{commonstartup}\VirtuaWin"; Filename: "{app}\VirtuaWin.exe"; Tasks: autostart
Name: "{group}\VirtuaWin"; Filename: "{app}\VirtuaWin.exe"
Name: "{group}\Help"; Filename: "{app}\VirtuaWin.hlp"
Name: "{group}\Readme"; Filename: "{app}\README.TXT"
Name: "{group}\History"; Filename: "{app}\HISTORY.TXT"
Name: "{group}\Uninstall"; Filename: "{uninstallexe}"

[UninstallDelete] 
Type: files; Name: "{app}\VWCONFIG.CFG"
Type: files; Name: "{app}\STICKY.CFG"
Type: files; Name: "{app}\TRICKY.CFG"
Type: files; Name: "{app}\USERLIST.CFG"
Type: files; Name: "{app}\VWSTATE.CFG"
Type: files; Name: "{app}\VWDISABLED.CFG"
Type: files; Name: "{app}\VWWINDOWSSTATE.CFG"

