#! /bin/bash

echo "Compiling YACReader"
cd ./YACReader
qmake "CONFIG+=release"
make
cd ..

echo "Compiling YACReaderLibrary"
cd ./YACReaderLibrary
qmake "CONFIG+=release"
make
cd ..

echo "Copying to destination folder"
dest='YACReader-'$1' X11-'$2'-qt4'
mkdir "$dest"
cp ./YACReader/YACReader "./${dest}/YACReader"
cp ./YACReaderLibrary/YACReaderLibrary "./${dest}/YACReaderLibrary"
cp ./COPYING.txt "./${dest}/"
cp ./README.txt "./${dest}/"
cp ./INSTALL.txt "./${dest}/"
cp ./images/icon.png "./${dest}/"
cp ./images/iconLibrary.png "./${dest}/"
cp ./images/db.png "./${dest}/"
cp ./images/coversPackage.png "./${dest}/"
cp -R ./utils "./${dest}/"
cp -R ./release/server "./${dest}/"
cp -R ./release/languages "./${dest}/"

echo "Creating package"
tar -czf "${dest}".tar.gz "${dest}"

echo "Done!"
