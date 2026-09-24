[Setup]
DefaultGroupName=YACReader
LanguageDetectionMethod=locale
AppId={{019AC70F-0312-76B8-BD8F-BE9DCBF49E25}
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
ArchitecturesInstallIn64BitMode={#PLATFORM}
ArchitecturesAllowed={#PLATFORM}
DisableWelcomePage=no
WizardImageFile=wizard_image_100.bmp,wizard_image_125.bmp,wizard_image_150.bmp,wizard_image_175.bmp,wizard_image_200.bmp,wizard_image_225.bmp,wizard_image_250.bmp
WizardSmallImageFile=wizard_small_image_100.bmp,wizard_small_image_125.bmp,wizard_small_image_150.bmp,wizard_small_image_175.bmp,wizard_small_image_200.bmp,wizard_small_image_225.bmp,wizard_small_image_250.bmp

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
Root: HKCR; Subkey: .ydb; ValueType: string; ValueData: YACReader Data Base (ydb); Flags: uninsdeletekey
Root: HKCR; SubKey: YACReader Data Base (ydb); ValueType: string; ValueData: YACReader Library Database; Flags: uninsdeletekey
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
Source: Qt6Qml.dll; DestDir: {app}
Source: Qt6QmlModels.dll; DestDir: {app}
Source: Qt6QmlMeta.dll; DestDir: {app}
Source: Qt6QmlWorkerScript.dll; DestDir: {app}
Source: Qt6Quick.dll; DestDir: {app}
Source: Qt6QuickEffects.dll; DestDir: {app}
Source: Qt6QuickControls2.dll; DestDir: {app}
Source: Qt6QuickControls2Impl.dll; DestDir: {app}
Source: Qt6QuickControls2Basic.dll; DestDir: {app}
Source: Qt6QuickControls2BasicStyleImpl.dll; DestDir: {app}
Source: Qt6QuickControls2Fusion.dll; DestDir: {app}
Source: Qt6QuickControls2FusionStyleImpl.dll; DestDir: {app}
Source: Qt6QuickLayouts.dll; DestDir: {app}
Source: Qt6QuickShapes.dll; DestDir: {app}
Source: Qt6QuickTemplates2.dll; DestDir: {app}
Source: Qt6QuickWidgets.dll; DestDir: {app}
Source: Qt6Sql.dll; DestDir: {app}
Source: Qt6Svg.dll; DestDir: {app}
Source: Qt6TextToSpeech.dll; DestDir: {app}

Source: opengl32sw.dll; DestDir: {app}; Flags: skipifsourcedoesntexist
Source: D3Dcompiler_47.dll; DestDir: {app}; Flags: skipifsourcedoesntexist

;Qt PlugIns
Source:generic\*;  DestDir: {app}\generic\
Source:iconengines\*;  DestDir: {app}\iconengines\
Source:imageformats\*;  DestDir: {app}\imageformats\
Source:networkinformation\*;  DestDir: {app}\networkinformation\
Source:platforms\*;  DestDir: {app}\platforms\
Source:qml\*;  DestDir: {app}\qml\; Flags: recursesubdirs
Source:qmltooling\*;  DestDir: {app}\qmltooling\
Source:sqldrivers\qsqlite.dll;  DestDir: {app}\sqldrivers\
Source:styles\*;  DestDir: {app}\styles\
Source:texttospeech\*;  DestDir: {app}\texttospeech\
Source:tls\*;  DestDir: {app}\tls\
Source:translations\*;  DestDir: {app}\translations\


;Libs
Source: pdfium.dll; DestDir: {app}
Source: openssl\*; DestDir: {app}

;vcredist
Source: "vc_redist.{#PLATFORM}.exe"; DestDir: {tmp}; Flags: deleteafterinstall

;Utils
;Source: utils\7zip.exe; DestDir: {app}\utils\
Source: utils\7z.dll; DestDir: {app}\utils\

;Bin
Source: YACReader.exe; DestDir: {app};
Source: YACReaderLibrary.exe; DestDir: {app}; Tasks:
Source: YACReaderLibraryServer.exe; DestDir: {app}; Tasks:

;License
Source: README.md; DestDir: {app}; Flags: isreadme
Source: COPYING.txt; DestDir: {app}

;Languages
Source: languages\*; DestDir: {app}\languages\; Flags: recursesubdirs; Excludes: "*_source.qm"
;Server
Source: server\*; DestDir: {app}\server\; Flags: recursesubdirs

[Dirs]
Name: {app};

[CustomMessages]
App=YACReader
AppLibrary=YACReaderLibrary
LaunchYACReaderLibrary=Start YACreaderLibrary after finishing installation
LaunchYACReader=Start YACreader after finishing installation

[Run]
Filename: {tmp}\vc_redist.{#PLATFORM}.exe; \
Parameters: "/install /quiet /norestart"; \
StatusMsg: "Installing VC++ Redistributables..."
Filename: "{sys}\netsh.exe"; Parameters: "advfirewall firewall add rule name=""YACReaderLibrary"" dir=in action=allow program=""{app}\YACReaderLibrary.exe"" enable=yes profile=any"; Flags: runhidden waituntilterminated

Filename: {app}\{cm:AppLibrary}.exe; Description: {cm:LaunchYACReaderLibrary,{cm:AppLibrary}}; Flags: nowait postinstall skipifsilent
Filename: {app}\{cm:App}.exe; Description: {cm:LaunchYACReader,{cm:App}}; Flags: nowait postinstall skipifsilent

[UninstallRun]
Filename: "{sys}\netsh.exe"; Parameters: "advfirewall firewall delete rule name=""YACReaderLibrary"""; Flags: runhidden waituntilterminated

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
procedure OpenWebPage(const URL: String);
var
  ErrorCode: Integer;
begin
  ShellExec('open', URL, '', '', SW_SHOWNORMAL, ewNoWait, ErrorCode);
end;

procedure IOSLinkOnClick(Sender: TObject);
begin
  OpenWebPage('https://apps.apple.com/app/id635717885');
end;

procedure AndroidLinkOnClick(Sender: TObject);
begin
  OpenWebPage('https://play.google.com/store/apps/details?id=com.yacreader.yacreader');
end;

procedure PatreonLinkOnClick(Sender: TObject);
begin
  OpenWebPage('https://www.patreon.com/yacreader');
end;

procedure PayPalLinkOnClick(Sender: TObject);
begin
  OpenWebPage('https://www.paypal.com/donate?business=5TAMNQCDDMVP8&item_name=Support+YACReader');
end;

// Adds a wrapped text line to the Welcome page, below the standard text
function AddWelcomeText(const Caption: String; Y: Integer; Bold: Boolean): TNewStaticText;
begin
  Result := TNewStaticText.Create(WizardForm);
  Result.Parent := WizardForm.WelcomePage;
  Result.AutoSize := False;
  Result.WordWrap := True;
  if Bold then
    Result.Font.Style := [fsBold];
  Result.SetBounds(WizardForm.WelcomeLabel2.Left, Y, WizardForm.WelcomeLabel2.Width, ScaleY(14));
  Result.Caption := Caption;
  Result.AdjustHeight();
end;

function AddWelcomeLink(const Caption: String; X, Y: Integer; OnClick: TNotifyEvent): TNewStaticText;
begin
  Result := TNewStaticText.Create(WizardForm);
  Result.Parent := WizardForm.WelcomePage;
  Result.Caption := Caption;
  Result.Left := X;
  Result.Top := Y;
  Result.Cursor := crHand;
  Result.Font.Style := [fsUnderline];
  Result.Font.Color := $CC6600;
  Result.OnClick := OnClick;
end;

function GetUninstallString(): String;
var
  sUnInstPath: String;
  sUnInstallString: String;
begin
  // First try the new AppId-based key
  sUnInstPath := ExpandConstant('Software\Microsoft\Windows\CurrentVersion\Uninstall\{#emit SetupSetting("AppId")}_is1');
  sUnInstallString := '';
  if not RegQueryStringValue(HKLM, sUnInstPath, 'UninstallString', sUnInstallString) then
    RegQueryStringValue(HKCU, sUnInstPath, 'UninstallString', sUnInstallString);
  
  // If not found, try the old AppName-based key (for versions without AppId)
  if sUnInstallString = '' then begin
    sUnInstPath := 'Software\Microsoft\Windows\CurrentVersion\Uninstall\YACReader_is1';
    if not RegQueryStringValue(HKLM, sUnInstPath, 'UninstallString', sUnInstallString) then
      RegQueryStringValue(HKCU, sUnInstPath, 'UninstallString', sUnInstallString);
  end;
  
  Result := sUnInstallString;
end;

function IsUpgrade(): Boolean;
begin
  Result := (GetUninstallString() <> '');
end;

function UnInstallOldVersion(): Integer;
var
  sUnInstallString: String;
  iResultCode: Integer;
begin
  Result := 0;
  sUnInstallString := GetUninstallString();
  if sUnInstallString <> '' then begin
    sUnInstallString := RemoveQuotes(sUnInstallString);
    if Exec(sUnInstallString, '/VERYSILENT /NORESTART /SUPPRESSMSGBOXES','', SW_HIDE, ewWaitUntilTerminated, iResultCode) then
      Result := 3
    else
      Result := 2;
  end else
    Result := 1;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if (CurStep=ssInstall) then
  begin
    if (IsUpgrade()) then
    begin
      UnInstallOldVersion();
    end;
  end;
end;

procedure InitializeWizard();
var
  InfoLabel: TNewStaticText;
  LinkLabel: TNewStaticText;
  LinkX: Integer;
  Y: Integer;
begin
  // Replace the standard Welcome text with one line and use the rest of the page for the mobile apps and donations
  WizardForm.WelcomeLabel2.Caption := 'This will install {#SetupSetting("AppVerName")} on your computer.';
  WizardForm.WelcomeLabel2.AdjustHeight();
  LinkX := WizardForm.WelcomeLabel2.Left;
  Y := WizardForm.WelcomeLabel2.Top + WizardForm.WelcomeLabel2.Height + ScaleY(20);

  InfoLabel := AddWelcomeText('Keep YACReader free and independent', Y, True);
  Y := InfoLabel.Top + InfoLabel.Height + ScaleY(4);
  InfoLabel := AddWelcomeText('YACReader is free and has no ads. Your support pays for new features and fixes.', Y, False);
  Y := InfoLabel.Top + InfoLabel.Height + ScaleY(6);
  LinkLabel := AddWelcomeLink('Become a patron', LinkX, Y, @PatreonLinkOnClick);
  AddWelcomeLink('Make a one-time donation', LinkLabel.Left + LinkLabel.Width + ScaleX(20), Y, @PayPalLinkOnClick);

  Y := LinkLabel.Top + LinkLabel.Height + ScaleY(20);
  InfoLabel := AddWelcomeText('Take your comics everywhere', Y, True);
  Y := InfoLabel.Top + InfoLabel.Height + ScaleY(4);
  InfoLabel := AddWelcomeText('YACReader for iPhone, iPad and Android connects to the library on this computer. Browse your collection over Wi-Fi, download comics to read offline, and keep your reading progress in sync.', Y, False);
  Y := InfoLabel.Top + InfoLabel.Height + ScaleY(6);
  LinkLabel := AddWelcomeLink('Get it on the App Store', LinkX, Y, @IOSLinkOnClick);
  AddWelcomeLink('Get it on Google Play', LinkLabel.Left + LinkLabel.Width + ScaleX(20), Y, @AndroidLinkOnClick);
end;
