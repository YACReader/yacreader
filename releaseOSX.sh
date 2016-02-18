#!/bin/bash

/Users/luisangel/my_dev/Qt5.5.1/5.5/clang_64/bin/macdeployqt YACReader.app
/Users/luisangel/my_dev/Qt5.5.1/5.5/clang_64/bin/macdeployqt YACReaderLibrary.app -qmldir=./YACReaderLibrary/qml
/Users/luisangel/my_dev/Qt5.5.1/5.5/clang_64/bin/macdeployqt YACReaderLibraryServer.app

cp -R ./utils ./YACReader.app/Contents/MacOS/
cp -R ./utils ./YACReaderLibrary.app/Contents/MacOS/
cp -R ./utils ./YACReaderLibraryServer.app/Contents/MacOS/
cp -R ./release/server ./YACReaderLibrary.app/Contents/MacOS/
cp -R ./release/server ./YACReaderLibraryServer.app/Contents/MacOS/
cp -R ./release/languages ./YACReader.app/Contents/MacOS/
cp -R ./release/languages ./YACReaderLibrary.app/Contents/MacOS/
cp -R ./release/languages ./YACReaderLibraryServer.app/Contents/MacOS/
/Users/luisangel/my_dev/YACReader/YACReader_hg/create-dmg

