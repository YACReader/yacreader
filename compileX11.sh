#! /bin/bash

cd ./compressed_archive
if [ ! -d "libp7zip" ]; then
echo "You need p7zip source code to compile YACReader. \
Please check the compressed_archive folder for further instructions."
exit
fi
patch -p0 -i libp7zip.patch
cd ..

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
dest='YACReader-'$1'-X11-'$2'-qt5'
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
