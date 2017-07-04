#!/bin/bash

hash macdeployqt 2>/dev/null || { echo >&2 "Qmake command not available. Please add the bin subfolder of your Qt installation  to the PATH environment variable."; exit 1; }

macdeployqt YACReader.app
macdeployqt YACReaderLibrary.app -qmldir=YACReaderLibrary/qml
macdeployqt YACReaderLibraryServer.app

#TODO: This copies unneeded stuff into the respective apps
#cp -R utils YACReader.app/Contents/MacOS/
#cp -R utils YACReaderLibrary.app/Contents/MacOS/
#cp -R utils YACReaderLibraryServer.app/Contents/MacOS/

mkdir -p YACReaderLibrary.app/Contents/MacOS/utils

cp -L /usr/local/bin/qrencode YACReaderLibrary.app/Contents/MacOS/utils/
#TODO: avoid using fixed paths here. Get the info from otool instead!

cp /usr/local/Cellar/qrencode/3.4.4/lib/libqrencode.3.dylib \
YACReaderLibrary.app/Contents/MacOS/utils

cp /usr/local/opt/libpng/lib/libpng16.16.dylib \
YACReaderLibrary.app/Contents/MacOS/utils

chmod +w YACReaderLibrary.app/Contents/MacOS/utils/*

install_name_tool -change /usr/local/Cellar/qrencode/3.4.4/lib/libqrencode.3.dylib \
@executable_path/utils/libqrencode.3.dylib \
YACReaderLibrary.app/Contents/MacOS/utils/qrencode

install_name_tool -change /usr/local/opt/libpng/libpng16.16.dylib \
@executable_path/utils/libpng16.16.dylib \
YACReaderLibrary.app/Contents/MacOS/utils/qrencode

cp -R release/server YACReaderLibrary.app/Contents/MacOS/
cp -R release/server YACReaderLibraryServer.app/Contents/MacOS/
cp -R release/languages YACReader.app/Contents/MacOS/
cp -R release/languages YACReaderLibrary.app/Contents/MacOS/
cp -R release/languages YACReaderLibraryServer.app/Contents/MacOS/

echo "Copied!"
#./signapps.sh
