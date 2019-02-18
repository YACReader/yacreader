#!/bin/bash

hash macdeployqt 2>/dev/null || { echo >&2 "macdeployqt command not available. Please add the bin subfolder of your Qt installation  to the PATH environment variable."; exit 1; }

echo "Preparing apps for release ..."

macdeployqt YACReader.app
macdeployqt YACReaderLibrary.app -qmldir=YACReaderLibrary/qml
macdeployqt YACReaderLibraryServer.app

mkdir -p YACReader.app/Contents/MacOS/utils
mkdir -p YACReaderLibrary.app/Contents/MacOS/utils
mkdir -p YACReaderLibraryServer.app/Contents/MacOS/utils

cp dependencies/qrencode/macx/libqrencode.4.0.0.dylib \
YACReaderLibrary.app/Contents/MacOS/utils/libqrencode.dylib

cp -R dependencies/7zip/macx/* YACReader.app/Contents/MacOS/utils/
cp -R dependencies/7zip/macx/* YACReaderLibrary.app/Contents/MacOS/utils/
cp -R dependencies/7zip/macx/* YACReaderLibraryServer.app/Contents/MacOS/utils/

cp -R release/server YACReaderLibrary.app/Contents/MacOS/
cp -R release/server YACReaderLibraryServer.app/Contents/MacOS/
cp -R release/languages YACReader.app/Contents/MacOS/
cp -R release/languages YACReaderLibrary.app/Contents/MacOS/
cp -R release/languages YACReaderLibraryServer.app/Contents/MacOS/

echo "Done."
#./signapps.sh
