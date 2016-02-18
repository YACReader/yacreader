#!/bin/bash

rm -R *.app
rm -R YACReader-*
rm -R *.dmg
cd YACReader
make clean
rm -R YACReader.app
cd ..
cd YACReaderLibrary
make clean
rm -R YACReaderLibrary.app
rm -R YACReaderLibraryServer.app
cd ..
