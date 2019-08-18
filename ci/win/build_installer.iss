[Setup]
DefaultGroupName=YACReader
LanguageDetectionMethod=locale
AppName=YACReader
AppVerName=YACReader v{#VERSION}
DefaultDirName={pf}\YACReader
OutputBaseFilename=YACReader-v{#VERSION}-win{#PLATFORM}-{#COMPRESSED_ARCHIVE_BACKEND}
LicenseFile=COPYING.txt
AlwaysUsePersonalGroup=true
OutputDir=..\Output
ChangesAssociations=true
SetupIconFile=setup.ico
UninstallDisplayIcon=uninstall.ico
;//TODO avoid setting ArchitecturesInstallIn64BitMode in 32bit installer
ArchitecturesInstallIn64BitMode={#PLATFORM}

[Registry]
Root: HKCR; SubKey: .cbz; ValueType: string; ValueData: Comic Book (zip); Flags: uninsdeletekey; Tasks: File_association
Root: HKCR; SubKey: Comic Book (zip); ValueType: string; ValueData: Comic Book file; Flags: uninsdeletekey; Tasks: File_association
Root: HKCR; SubKey: Comic Book (zip)\Shell\Open\Command; ValueType: string; ValueData: """{app}\YACReader.exe"" ""%1"""; Flags: uninsdeletevalue; Tasks: File_association
Root: HKCR; Subkey: Comic Book (zip)\DefaultIcon; ValueType: string; ValueData: {app}\YACReader.exe,0; Flags: uninsdeletevalue; Tasks: File_association
Root: HKCR; SubKey: .cbr; ValueType: string; ValueData: Comic Book (rar); Flags: uninsdeletekey; Tasks: File_association
Root: HKCR; SubKey: Comic Book (rar); ValueType: string; ValueData: Comic Book file; Flags: uninsdeletekey; Tasks: File_association
Root: HKCR; SubKey: Comic Book (rar)\Shell\Open\Command; ValueType: string; ValueData: """{app}\YACReader.exe"" ""%1"""; Flags: uninsdeletevalue; Tasks: File_association
Root: HKCR; Subkey: Comic Book (rar)\DefaultIcon; ValueType: string; ValueData: {app}\YACReader.exe,0; Flags: uninsdeletevalue; Tasks: File_association
Root: HKCR; Subkey: .clc; ValueType: string; ValueData: Compressed Library Covers (clc); Flags: uninsdeletekey
Root: HKCR; SubKey: Compressed Library Covers (clc); ValueType: string; ValueData: Compressed Library Covers; Flags: uninsdeletekey
Root: HKCR; Subkey: Compressed Library Covers (clc)\DefaultIcon; ValueType: string; ValueData: {app}\YACReaderLibrary.exe,1; Flags: uninsdeletevalue
Root: HKCR; Subkey: .ydb; ValueType: string; ValueData: Compressed Library Covers (clc); Flags: uninsdeletekey
Root: HKCR; SubKey: YACReader Data Base (ydb); ValueType: string; ValueData: Compressed Library Covers; Flags: uninsdeletekey
Root: HKCR; Subkey: YACReader Data Base (ydb)\DefaultIcon; ValueType: string; ValueData: {app}\YACReaderLibrary.exe,1; Flags: uninsdeletevalue

[Files]
;Qt Frameworks
Source: Qt5Core.dll; DestDir: {app}
Source: Qt5Gui.dll; DestDir: {app}
Source: Qt5Multimedia.dll; DestDir: {app}
Source: Qt5Network.dll; DestDir: {app}
Source: Qt5Qml.dll; DestDir: {app}
Source: Qt5Quick.dll; DestDir: {app}
Source: Qt5Script.dll; DestDir: {app}
Source: Qt5Sql.dll; DestDir: {app}
Source: Qt5Svg.dll; DestDir: {app}
Source: Qt5Widgets.dll; DestDir: {app}
Source: Qt5QuickWidgets.dll; DestDir: {app}

;Qt Angle
Source: D3Dcompiler_47.dll; DestDir: {app}
Source: libEGL.dll; DestDir: {app}
Source: libGLESV2.dll; DestDir: {app}
Source: opengl32sw.dll; DestDir: {app}

;Qt QML
Source: QtQml\*; DestDir: {app}\QtQml\; Flags: recursesubdirs
Source: QtQuick\*; DestDir: {app}\QtQuick\; Flags: recursesubdirs
Source: QtQuick.2\*; DestDir: {app}\QtQuick.2\; Flags: recursesubdirs
Source: QtGraphicalEffects\*; DestDir: {app}\QtGraphicalEffects\; Flags: recursesubdirs

;Qt PlugIns
Source:audio\*;  DestDir: {app}\audio\
Source:bearer\*;  DestDir: {app}\bearer\
Source:iconengines\*;  DestDir: {app}\iconengines\
Source:imageformats\*;  DestDir: {app}\imageformats\
Source:mediaservice\*;  DestDir: {app}\mediaservice\
Source:platforms\*;  DestDir: {app}\platforms\
Source:playlistformats\*;  DestDir: {app}\playlistformats\     
Source:qmltooling\*;  DestDir: {app}\qmltooling\          
Source:scenegraph\*;  DestDir: {app}\scenegraph\                         
Source:sqldrivers\qsqlite.dll;  DestDir: {app}\sqldrivers\
Source:translations\*;  DestDir: {app}\translations\    
Source:styles\*;  DestDir: {app}\styles\            

;Libs
Source: pdfium.dll; DestDir: {app}
Source: qrencode.dll; DestDir: {app}
Source: libeay32.dll; DestDir: {app}
Source: ssleay32.dll; DestDir: {app}

;vcredist
Source: "vc_redist.{#PLATFORM}.exe"; DestDir: {tmp}; Flags: deleteafterinstall

;Utils
;Source: utils\7zip.exe; DestDir: {app}\utils\
Source: utils\7z.dll; DestDir: {app}\utils\

;Bin
Source: YACReader.exe; DestDir: {app}; Permissions: everyone-full
Source: YACReaderLibrary.exe; DestDir: {app}; Permissions: everyone-full; Tasks:
Source: YACReaderLibraryServer.exe; DestDir: {app}; Permissions: everyone-full; Tasks:
 
;License
Source: README.md; DestDir: {app}; Flags: isreadme
Source: COPYING.txt; DestDir: {app}

;Languages
Source: languages\*; DestDir: {app}\languages\; Flags: recursesubdirs
;Server
Source: server\*; DestDir: {app}\server\; Flags: recursesubdirs

[Dirs]
Name: {app}; Permissions: everyone-full

[CustomMessages]
App=YACReader
AppLibrary=YACReaderLibrary
LaunchYACReaderLibrary=Start YACreaderLibrary after finishing installation
LaunchYACReader=Start YACreader after finishing installation

[Run]
Filename: {tmp}\vc_redist.{#PLATFORM}.exe; \
Parameters: "/q /passive /norestart /Q:a /c:""msiexec /q /i vcredist.msi"""; \
StatusMsg: "Installing VC++ Redistributables..."

Filename: {app}\{cm:AppLibrary}.exe; Description: {cm:LaunchYACReaderLibrary,{cm:AppLibrary}}; Flags: nowait postinstall skipifsilent
Filename: {app}\{cm:App}.exe; Description: {cm:LaunchYACReader,{cm:App}}; Flags: nowait postinstall skipifsilent

[Icons]
Name: {group}\YACReader; Filename: {app}\YACReader.exe; WorkingDir: {app}; IconIndex: 0
Name: {group}\YACReader Library; Filename: {app}\YACReaderLibrary.exe; WorkingDir: {app}; IconIndex: 0
;Name: {group}\YACReader Library Package; Filename: {app}\YACReaderLibrary.exe; WorkingDir: {app}; IconIndex: 0
[Tasks]
Name: File_association; Description: Associate .cbz and .cbr files with YACReader
;Name: StartYACReaderAfterInstall; Description: Run YACReader after install
;Name: StartYACReaderLibraryAfterInstall; Description: Run YACReaderLibrary after install

[ThirdPartySettings]
CompileLogMethod=append

[Code]
var donationPage: TOutputMsgWizardPage;
var URLLabel: TNewStaticText;

procedure URLLabelOnClick(Sender: TObject);
var ErrorCode: Integer;
begin
ShellExec('open', 'https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=5TAMNQCDDMVP8&item_name=YACReader&currency_code=EUR&bn=PP%2dDonationsBF%3abtn_donate_LG%2egif%3aNonHosted', '', '', SW_SHOWNORMAL, ewNoWait, ErrorCode);
end;

procedure InitializeWizard();

begin

  URLLabel := TNewStaticText.Create(WizardForm);
  URLLabel.Caption:='Make a DONATION/Haz una DONACIÓN';
  URLLabel.Cursor:=crHand;
  URLLabel.OnClick:=@URLLabelOnClick;
  URLLabel.Parent:=WizardForm;
  // Alter Font
  URLLabel.Font.Style:=URLLabel.Font.Style + [fsUnderline];
  URLLabel.Font.Color:=clBlue;
  URLLabel.Top:=250;

  URLLabel.Left:=35;

donationPage := CreateOutputMsgPage(wpWelcome,
  'Iformation', 'Please read the following information before continuing.',
  'YACReader is FREE software. If you like it, please, consider to make a DONATION'#13#13 +
  'YACReader es software libre y GRATUITO. Si te gusta, por favor, considera realizar una DONACIÓN'#13#13)

end;

procedure CurPageChanged(CurPageID: Integer);
begin
if CurPageID=donationPage.ID then URLLabel.Visible:=true else URLLabel.Visible:=false;
end;
