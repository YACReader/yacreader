#! /bin/bash

echo "Compiling YACReader"
cd ./YACReader
qmake -spec macx-g++
make
cd ..

echo "Compiling YACReaderLibrary"
cd ./YACReaderLibrary
qmake -spec macx-g++
make
cd ..

mkdir ./bin
cp -R ./YACReader/YACReader.app ./YACReader.app
cp -R ./YACReaderLibrary/YACReaderLibrary.app ./YACReaderLibrary.app

echo "Configuring release apps"
./releaseOSX.sh

echo "Done!"
