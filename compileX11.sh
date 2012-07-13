#! /bin/bash

echo "Compiling YACReader"
cd ./YACReader
qmake
make
cd ..

echo "Compiling YACReaderLibrary"
cd ./YACReaderLibrary
qmake
make
cd ..

mkdir ./bin
cp ./YACReader/YACReader ./bin/YACReader
cp ./YACReaderLibrary/YACReaderLibrary ./bin/YACReaderLibrary

echo "Done!"
