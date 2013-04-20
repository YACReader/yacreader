
echo Setting up a Qt environment...

set QTDIR=D:\Desarrollo\Qt\4.8.3
echo -- QTDIR set to D:\Desarrollo\Qt\4.8.3
set PATH=D:\Desarrollo\Qt\4.8.3\bin;%PATH%
echo -- Added D:\Desarrollo\Qt\4.8.3\bin to PATH
set QMAKESPEC=win32-msvc2010
echo -- QMAKESPEC set to "win32-msvc2010"

if not "%1"=="vsvars" goto ENDVSVARS
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\Tools\vsvars32.bat"
:ENDVSVARS

if not "%1"=="vsstart" goto ENDVSSTART
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\Tools\vsvars32.bat"
devenv /useenv
:ENDVSSTART


cd YACReader
echo Entering YACReader
qmake -spec win32-msvc2010 -tp vc YACReader.pro
echo qmake -spec win32-msvc2010 -tp vc YACReader.pro
cd ..
echo leaving YACReader
cd YACReaderLibrary
echo Entering YACReaderLibrary
qmake -spec win32-msvc2010 -tp vc YACReaderLibrary.pro
echo qmake -spec win32-msvc2010 -tp vc YACReaderLibrary.pro
cd ..