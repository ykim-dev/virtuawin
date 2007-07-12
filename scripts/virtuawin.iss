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
Name: en; MessagesFile: VirtuaWin5.0.ISL

[Tasks]
Name: autostart; Description: Run automatically on Windows startup
Name: userpath; Description: Setup the user configuration path for roaming profile or portable app support ; Flags: unchecked

[Dirs]
Name: {app}\icons; Flags: uninsalwaysuninstall
Name: {app}\modules; Flags: uninsalwaysuninstall

[Files]
Source: VirtuaWin.exe; DestDir: {app}
Source: VirtuaWin.hlp; DestDir: {app}
Source: WinList.exe; DestDir: {app}\Modules\
Source: VWAssigner.exe; DestDir: {app}\Modules\
Source: UserList.cfg; DestDir: {app}; Flags: onlyifdoesntexist
Source: Tricky.cfg; DestDir: {app}; Flags: onlyifdoesntexist
Source: COPYING.TXT; DestDir: {app}
Source: HISTORY.TXT; DestDir: {app}
Source: README.TXT; DestDir: {app}; Flags: isreadme

[Icons]
Name: {commonstartup}\VirtuaWin; Filename: {app}\VirtuaWin.exe; Tasks: autostart
Name: {group}\VirtuaWin; Filename: {app}\VirtuaWin.exe
Name: {group}\Help; Filename: {app}\VirtuaWin.hlp
Name: {group}\Readme; Filename: {app}\README.TXT
Name: {group}\History; Filename: {app}\HISTORY.TXT
Name: {group}\Uninstall; Filename: {uninstallexe}

[Run]
Filename: {app}\VirtuaWin.exe; WorkingDir: {app}; Description: {cm:LaunchProgram,VirtuaWin}; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: files; Name: {app}\VWCONFIG.CFG
Type: files; Name: {app}\STICKY.CFG
Type: files; Name: {app}\TRICKY.CFG
Type: files; Name: {app}\USERLIST.CFG
Type: files; Name: {app}\VWSTATE.CFG
Type: files; Name: {app}\VWDISABLED.CFG
Type: files; Name: {app}\VWWINDOWSSTATE.CFG

[Code]
var
  UserPathPage: TInputQueryWizardPage;

procedure InitializeWizard;
begin
  { Create the pages }

  UserPathPage := CreateInputQueryPage(wpSelectTasks,
    'Addition Tasks', 'Setup the user configuration path.',
    'Please specify the path you want your configuration files to be stored, then click Next.'#13#13 +
    'You can use this option to solve problems with roaming profiles or to make the VirtuaWin installation portable. ' +
    'This will create a userpath.cfg file in the VirtuaWin installation path.'#13#13 + 
    'The path can use environment variables (e.g. "${USERNAME}" for your login user name), ' +
    'use "${VIRTUAWIN_PATH}" for the VirtuaWin installation path. The default path used by VirtuaWin is "${APPDATA}\VirtuaWin".');
  UserPathPage.Add('User Path:', False);

  { Set default values, using settings that were stored last time if possible }

  UserPathPage.Values[0] := GetPreviousData('UserPath', '${VIRTUAWIN_PATH}\config\${USERNAME}');
end;

procedure RegisterPreviousData(PreviousDataKey: Integer);
begin
  { Store the settings so we can restore them next time }
  SetPreviousData(PreviousDataKey, 'UserPath', UserPathPage.Values[0]);
end;

function DoUserPath(): Boolean;
var
  ss: String;
begin
  ss := WizardSelectedTasks(False);
  Result := Pos('userpath', ss) <> 0;
end;

function ShouldSkipPage(PageID: Integer): Boolean;
begin
  if (PageID = UserPathPage.ID) then
    Result := not DoUserPath()
  else
    Result := False;
end;

procedure CurStepChanged(CurStep: TSetupStep);
var
  ss: String;
begin
  if (CurStep = ssPostInstall) and DoUserPath() then begin
    ss := UserPathPage.Values[0]
    if (Length(ss) > 0) then begin
      SaveStringToFile(ExpandConstant('{app}') + '\userpath.cfg',ss + #13#10, False);
    end else begin
      DeleteFile(ExpandConstant('{app}') + '\userpath.cfg')
    end ;
  end ;
end;
