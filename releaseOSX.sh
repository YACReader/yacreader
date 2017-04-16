#!/bin/bash

hash macdeployqt 2>/dev/null || { echo >&2 "Qmake command not available. Please add the bin subfolder of your Qt installation  to the PATH environment variable."; exit 1; }

macdeployqt YACReader.app
macdeployqt YACReaderLibrary.app -qmldir=YACReaderLibrary/qml
macdeployqt YACReaderLibraryServer.app

#TODO: This copies unneeded stuff into the respective apps
cp -R utils YACReader.app/Contents/MacOS/
cp -R utils YACReaderLibrary.app/Contents/MacOS/
cp -R utils YACReaderLibraryServer.app/Contents/MacOS/

cp -R release/server YACReaderLibrary.app/Contents/MacOS/
cp -R release/server YACReaderLibraryServer.app/Contents/MacOS/
cp -R release/languages YACReader.app/Contents/MacOS/
cp -R release/languages YACReaderLibrary.app/Contents/MacOS/
cp -R release/languages YACReaderLibraryServer.app/Contents/MacOS/

#./signapps.sh
