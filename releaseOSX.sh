#!/bin/bash

/Users/luisangel/my_dev/Qt5.5.0/5.5/clang_64/bin/macdeployqt YACReader.app
/Users/luisangel/my_dev/Qt5.5.0/5.5/clang_64/bin/macdeployqt YACReaderLibrary.app -qmldir=./YACReaderLibrary/qml

#macdeployqt YACReader.app
#macdeployqt YACReaderLibrary.app

cp -R ./utils ./YACReader.app/Contents/MacOS/
cp -R ./utils ./YACReaderLibrary.app/Contents/MacOS/
cp -R ./release/server ./YACReaderLibrary.app/Contents/MacOS/
cp -R ./release/languages ./YACReader.app/Contents/MacOS/
cp -R ./release/languages ./YACReaderLibrary.app/Contents/MacOS/
#cd ./YACReaderLibrary.app/Contents/MacOS/
#touch YACReaderLibrary.ini
#ln -s ../../../YACReader.app/Contents/MacOS/YACReader

