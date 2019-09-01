#! /bin/bash
set -e

VERSION=${1:-"9.5.0"}

BUILD_NUMBER=${2:-"0"}

if [ "$3" == "clean" ]; then
./cleanOSX.sh
fi

hash qmake 2>/dev/null || { echo >&2 "Qmake command not available. Please add the bin subfolder of your Qt installation to the PATH environment variable."; exit 1; }

echo "Compiling YACReader"
cd YACReader
qmake DEFINES+="BUILD_NUMBER=\\\\\\\"${BUILD_NUMBER}\\\\\\\""
make
cd ..

echo "Compiling YACReaderLibrary"
cd YACReaderLibrary
qmake DEFINES+="BUILD_NUMBER=\\\\\\\"${BUILD_NUMBER}\\\\\\\""
make
cd ..

echo "Compiling YACReaderLibraryServer"
cd YACReaderLibraryServer
qmake DEFINES+="BUILD_NUMBER=\\\\\\\"${BUILD_NUMBER}\\\\\\\""
make
cd ..

echo "Configuring release apps"

cp -R YACReader/YACReader.app YACReader.app
cp -R YACReaderLibrary/YACReaderLibrary.app YACReaderLibrary.app
cp -R YACReaderLibraryServer/YACReaderLibraryServer.app YACReaderLibraryServer.app

./releaseOSX.sh

echo "Copying to destination folder"
dest="YACReader-$VERSION.$BUILD_NUMBER MacOSX-Intel"
mkdir -p "$dest"
cp -R YACReader.app "${dest}/YACReader.app"
cp -R YACReaderLibrary.app "${dest}/YACReaderLibrary.app"
cp -R YACReaderLibraryServer.app "${dest}/YACReaderLibraryServer"

cp COPYING.txt "${dest}/"
cp README.md "${dest}/"

#mkdir -p "${dest}/icons/"
#cp images/db.png "${dest}/icons/"
#cp images/coversPackage.png "${dest}/icons/"

echo "Creating dmg package"
#tar -czf "${dest}".tar.gz "${dest}"
#hdiutil create "${dest}".dmg -srcfolder "./${dest}" -ov

#create-dmg --volname "YACReader $VERSION.$BUILD_NUMBER Installer" --volicon icon.icns --window-size 600 403 --icon-size 128 --app-drop-link 485 90 --background background.png --icon YACReader 80 90 --icon YACReaderLibrary 235 90 --eula COPYING.txt --icon YACReaderLibraryServer 470 295 --icon README.md 120 295 --icon COPYING.txt 290 295 "$dest.dmg" "$dest"

sed -i'' -e 's/#VERSION#/${VERSION}/g' dmg.json
sed -i'' -e 's/#BUILD_NUMBER#/${BUILD_NUMBER}/g' dmg.json
appdmg dmg.json "$dest.dmg"

echo "Done!"
