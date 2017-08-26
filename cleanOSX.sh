#!/bin/bash

rm -R *.app
rm -R YACReader-*
rm -R *.dmg
cd YACReader
make distclean
rm -R YACReader.app
cd ..
cd YACReaderLibrary
make distclean
rm -R YACReaderLibrary.app
cd ..
cd YACReaderLibraryServer
make distclean
rm -R YACReaderLibraryServer.app
cd ..
