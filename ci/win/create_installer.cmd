@echo off

SET src_path=..\..\..

set ARCH=%1
set COMPRESSION=%2
set BUILD_NUMBER=%3

SET exe_path=%src_path%\build\bin

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

rem Collect cmake-generated .qm translation files from the build tree
rem (release\languages is not tracked in git; cmake generates .qm in build subdirs)
mkdir languages
for /r %src_path%\build %%f in (*.qm) do (
    echo %%~nf | findstr /I /R "_source$" >nul
    if errorlevel 1 copy "%%f" .\languages\ >nul
)

copy %src_path%\vc_redist.%ARCH%.exe .

type %src_path%\common\yacreader_global.h | findstr /R /C:"#define VERSION " > tmp
set /p VERSION= < tmp
set VERSION=%VERSION:#define VERSION "=%
set VERSION=%VERSION:"=%
echo %VERSION%
del tmp

echo "iscc start"
iscc /DVERSION=%VERSION% /DPLATFORM=%ARCH% /DCOMPRESSED_ARCHIVE_BACKEND=%COMPRESSION% /DBUILD_NUMBER=%BUILD_NUMBER% build_installer_qt6.iss || exit /b
echo "iscc done!"

cd ..
