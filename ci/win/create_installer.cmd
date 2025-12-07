@echo off

SET src_path=..\..\..

set ARCH=%1
set COMPRESSION=%2
set BUILD_NUMBER=%3
set QT_VERSION=%4

IF "%ARCH%"=="x64" (
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

windeployqt --release -qml YACReader.exe
windeployqt --release -qml --qmldir %src_path%\YACReaderLibrary\qml YACReaderLibrary.exe
windeployqt YACReaderLibraryServer.exe

mkdir utils

IF "%COMPRESSION%"=="7z" (
    copy %src_path%\dependencies\7zip\win\%ARCH%\7z.dll .\utils\7z.dll
) ELSE (
    copy %src_path%\dependencies\unarr\win\%ARCH%\unarr.dll .
)

copy %src_path%\dependencies\pdfium\win\%ARCH%\pdfium.dll .

mkdir openssl
copy %src_path%\dependencies\openssl\win\%ARCH%\* .\openssl\

xcopy %src_path%\release\server .\server /i /e
xcopy %src_path%\release\languages .\languages /i /e

copy %src_path%\vc_redist.%ARCH%.exe .

type %src_path%\common\yacreader_global.h | findstr /R /C:"#define VERSION " > tmp
set /p VERSION= < tmp
set VERSION=%VERSION:#define VERSION "=%
set VERSION=%VERSION:"=%
echo %VERSION%
del tmp

if "%ARCH%"=="x86" (
    type build_installer.iss | findstr /v ArchitecturesInstallIn64BitMode | findstr /v ArchitecturesAllowed > copy_build_installer.iss
    type copy_build_installer.iss > build_installer.iss
)

echo "iscc start"
if "%QT_VERSION%"=="qt6" (
	iscc /DVERSION=%VERSION% /DPLATFORM=%ARCH% /DCOMPRESSED_ARCHIVE_BACKEND=%COMPRESSION% /DBUILD_NUMBER=%BUILD_NUMBER% build_installer_qt6.iss || exit /b
) else (
	iscc /DVERSION=%VERSION% /DPLATFORM=%ARCH% /DCOMPRESSED_ARCHIVE_BACKEND=%COMPRESSION% /DBUILD_NUMBER=%BUILD_NUMBER% build_installer.iss || exit /b
)
echo "iscc done!"

cd ..
