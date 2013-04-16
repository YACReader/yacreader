#!/bin/bash
macdeployqt YACReader.app
macdeployqt YACReaderLibrary.app
cp -R ./utils ./YACReader.app/Contents/MacOS/
cp -R ./utils ./YACReaderLibrary.app/Contents/MacOS/
cp -R ./release/server ./YACReaderLibrary.app/Contents/MacOS/
cd ./YACReaderLibrary.app/Contents/MacOS/
ln -s ../../../YACReader.app/Contents/MacOS/YACReader

