#!/bin/bash

/Users/luisangel/Qt/5.3/clang_64/bin/macdeployqt YACReader.app
/Users/luisangel/Qt/5.3/clang_64/bin/macdeployqt YACReaderLibrary.app

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

