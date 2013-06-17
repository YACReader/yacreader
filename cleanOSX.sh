#!/bin/bash

rm -R *.app
rm -R YACReader-*
cd YACReader
make clean
rm -R YACReader.app
cd ..
cd YACReaderLibrary
make clean
rm -R YACReaderLibrary.app
cd ..
