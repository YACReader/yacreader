@echo off

SET src_path=..\..\..

IF "%1"=="x64" (
	SET exe_path=%src_path%\release64
) ELSE (
	SET exe_path=%src_path%\release
)

rmdir /S /Q installer_contents

mkdir installer_contents

copy * installer_contents

cd installer_contents

copy %src_path%\COPYING.txt .
copy %src_path%\README.md .

copy %exe_path%\YACReader.exe .
copy %exe_path%\YACReaderLibrary.exe .
copy %exe_path%\YACReaderLibraryServer.exe .

windeployqt --release YACReader.exe
windeployqt --release --qmldir %src_path%\YACReaderLibrary\qml YACReaderLibrary.exe
windeployqt YACReaderLibraryServer.exe

mkdir utils

IF "%2"=="7z" (
	copy %src_path%\dependencies\7zip\win\%1\7z.dll .\utils\7z.dll
) ELSE (
    copy %src_path%\dependencies\unarr\win\%1\unarr.dll .
)

mkdir openssl

copy %src_path%\dependencies\qrencode\win\%1\qrencode.dll .
copy %src_path%\dependencies\pdfium\win\%1\pdfium.dll .
copy %src_path%\dependencies\openssl\win\%1\* .\openssl\

xcopy %src_path%\release\server .\server /i /e
xcopy %src_path%\release\languages .\languages /i /e

copy %src_path%\vc_redist.%1.exe .

type %src_path%\common\yacreader_global.h | findstr /R /C:"#define VERSION " > tmp
set /p VERSION= < tmp
set VERSION=%VERSION:#define VERSION "=%
set VERSION=%VERSION:"=%
echo %VERSION%
del

if "%1"=="x86" (
	type build_installer.iss | findstr /v ArchitecturesInstallIn64BitMode | findstr /v ArchitecturesAllowed > copy_build_installer.iss
	type copy_build_installer.iss > build_installer.iss
)

if "%4"=="qt6" (
	iscc /DVERSION=%VERSION% /DPLATFORM=%1 /DCOMPRESSED_ARCHIVE_BACKEND=%2 /DBUILD_NUMBER=%3 build_installer_qt6.iss "/Ssigntool=signtool.exe sign /f %5 /p %6 $f" || exit /b
) else (
	iscc /DVERSION=%VERSION% /DPLATFORM=%1 /DCOMPRESSED_ARCHIVE_BACKEND=%2 /DBUILD_NUMBER=%3 build_installer.iss "/Ssigntool=signtool.exe sign /f %5 /p %6 $f" || exit /b
)

cd ..