[Setup]
DefaultGroupName=YACReader
LanguageDetectionMethod=locale
AppName=YACReader
AppVerName=YACReader v{#VERSION}.{#BUILD_NUMBER}
AppVersion={#VERSION}.{#BUILD_NUMBER}
VersionInfoVersion={#VERSION}
DefaultDirName={pf}\YACReader
OutputBaseFilename=YACReader-v{#VERSION}.{#BUILD_NUMBER}-win{#PLATFORM}-{#COMPRESSED_ARCHIVE_BACKEND}-qt6
LicenseFile=COPYING.txt
AlwaysUsePersonalGroup=true
OutputDir=..\Output
ChangesAssociations=true
SetupIconFile=setup.ico
UninstallDisplayIcon=uninstall.ico
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64
#if CODE_SIGN == "true"
  SignTool=signtool
#endif

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
Source: Qt6Widgets.dll; DestDir: {app}
Source: Qt6Core.dll; DestDir: {app}
Source: Qt6Core5Compat.dll; DestDir: {app}
Source: Qt6Gui.dll; DestDir: {app}
Source: Qt6Multimedia.dll; DestDir: {app}
Source: Qt6Network.dll; DestDir: {app}
Source: Qt6OpenGL.dll; DestDir: {app}
Source: Qt6OpenGLWidgets.dll; DestDir: {app}
Source: Qt6Qml.dll; DestDir: {app}
Source: Qt6QmlLocalStorage.dll; DestDir: {app}
Source: Qt6QmlModels.dll; DestDir: {app}
Source: Qt6QmlWorkerScript.dll; DestDir: {app}
Source: Qt6QmlXmlListModel.dll; DestDir: {app}
Source: Qt6Quick.dll; DestDir: {app}
Source: Qt6QuickControls2.dll; DestDir: {app}
Source: Qt6QuickControls2Impl.dll; DestDir: {app}
Source: Qt6QuickDialogs2.dll; DestDir: {app}
Source: Qt6QuickDialogs2QuickImpl.dll; DestDir: {app}
Source: Qt6QuickDialogs2Utils.dll; DestDir: {app}
Source: Qt6QuickLayouts.dll; DestDir: {app}
Source: Qt6QuickParticles.dll; DestDir: {app}
Source: Qt6QuickShapes.dll; DestDir: {app}
Source: Qt6QuickTemplates2.dll; DestDir: {app}
Source: Qt6QuickWidgets.dll; DestDir: {app}
Source: Qt6Sql.dll; DestDir: {app}
Source: Qt6Svg.dll; DestDir: {app}


;Qt Angle
Source: opengl32sw.dll; DestDir: {app}
Source: D3Dcompiler_47.dll; DestDir: {app}

;Qt QML
Source: QtQml\*; DestDir: {app}\QtQml\; Flags: recursesubdirs
Source: QtQuick\*; DestDir: {app}\QtQuick\; Flags: recursesubdirs

;Qt5 Compat
Source: Qt5Compat\*; DestDir: {app}\Qt5Compat\; Flags: recursesubdirs

;Qt PlugIns
Source:iconengines\*;  DestDir: {app}\iconengines\
Source:imageformats\*;  DestDir: {app}\imageformats\
Source:networkinformation\*;  DestDir: {app}\networkinformation\
Source:platforms\*;  DestDir: {app}\platforms\
Source:qmltooling\*;  DestDir: {app}\qmltooling\   
Source:sqldrivers\qsqlite.dll;  DestDir: {app}\sqldrivers\
Source:styles\*;  DestDir: {app}\styles\
Source:tls\*;  DestDir: {app}\tls\
Source:translations\*;  DestDir: {app}\translations\    
            

;Libs
Source: pdfium.dll; DestDir: {app}
Source: qrencode.dll; DestDir: {app}
Source: openssl\*; DestDir: {app}

;vcredist
Source: "vc_redist.{#PLATFORM}.exe"; DestDir: {tmp}; Flags: deleteafterinstall

;Utils
;Source: utils\7zip.exe; DestDir: {app}\utils\
Source: utils\7z.dll; DestDir: {app}\utils\

;Bin
Source: YACReader.exe; DestDir: {app}; Permissions: everyone-full;
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
Parameters: "/uninstall /quiet /norestart"; \
StatusMsg: "Uninstalling VC++ Redistributables..."

Filename: {tmp}\vc_redist.{#PLATFORM}.exe; \
Parameters: "/install /quiet /norestart"; \
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
  'YACReader es software libre y GRATUITO. Si te gusta, por favor, considera realizar una DONACIï¿½N'#13#13)

end;

procedure CurPageChanged(CurPageID: Integer);
begin
if CurPageID=donationPage.ID then URLLabel.Visible:=true else URLLabel.Visible:=false;
end;
