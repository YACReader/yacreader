#! /bin/bash

VERSION=${1:-"9.5.0"}

if [ "$2" == "clean" ]; then
./cleanOSX.sh
fi

hash qmake 2>/dev/null || { echo >&2 "Qmake command not available. Please add the bin subfolder of your Qt installation to the PATH environment variable."; exit 1; }

echo "Compiling YACReader"
cd YACReader
qmake
make
cd ..

echo "Compiling YACReaderLibrary"
cd YACReaderLibrary
qmake
make
cd ..

echo "Compiling YACReaderLibraryServer"
cd YACReaderLibraryServer
qmake
make
cd ..

echo "Configuring release apps"

cp -R YACReader/YACReader.app YACReader.app
cp -R YACReaderLibrary/YACReaderLibrary.app YACReaderLibrary.app
cp -R YACReaderLibraryServer/YACReaderLibraryServer.app YACReaderLibraryServer.app

./releaseOSX.sh

echo "Copying to destination folder"
dest="YACReader-$VERSION MacOSX-Intel"
mkdir -p "$dest"
cp -R YACReader.app "${dest}/YACReader.app"
cp -R YACReaderLibrary.app "${dest}/YACReaderLibrary.app"
cp -R YACReaderLibraryServer.app "${dest}/YACReaderLibraryServer"

cp COPYING.txt "${dest}/"
cp README.txt "${dest}/"

#mkdir -p "${dest}/icons/"
#cp images/db.png "${dest}/icons/"
#cp images/coversPackage.png "${dest}/icons/"

echo "Creating dmg package"
#tar -czf "${dest}".tar.gz "${dest}"
#hdiutil create "${dest}".dmg -srcfolder "./${dest}" -ov
./dependencies/create-dmg/create-dmg --volname "YACReader $VERSION Installer" --volicon icon.icns --window-size 600 403 --icon-size 128 --app-drop-link 485 90 --background background.png --icon YACReader 80 90 --icon YACReaderLibrary 235 90 --eula COPYING.txt --icon YACReaderLibraryServer 470 295 --icon README.txt 120 295 --icon COPYING.txt 290 295 "$dest.dmg" "$dest"

echo "Done!"
