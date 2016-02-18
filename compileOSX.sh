#! /bin/bash
if [ $2 == "clean" ]; then
./cleanOSX.sh
fi

echo "Compiling YACReader"
cd ./YACReader
/Users/luisangel/my_dev/Qt5.5.1/5.5/clang_64/bin/qmake YACReader.pro  -spec macx-clang "CONFIG+=release"
make
cd ..

echo "Compiling YACReaderLibrary"
cd ./YACReaderLibrary
/Users/luisangel/my_dev/Qt5.5.1/5.5/clang_64/bin/qmake YACReaderLibrary.pro -spec macx-clang "CONFIG+=release"
make
cd ..

echo "Compiling YACReaderLibraryServer"
cd ./YACReaderLibrary
make clean
/Users/luisangel/my_dev/Qt5.5.1/5.5/clang_64/bin/qmake YACReaderLibraryServer.pro -spec macx-clang "CONFIG+=release"
make
cd ..

echo "Configuring release apps"

cp -R ./YACReader/YACReader.app ./YACReader.app
cp -R ./YACReaderLibrary/YACReaderLibrary.app ./YACReaderLibrary.app
cp -R ./YACReaderLibrary/YACReaderLibraryServer.app ./YACReaderLibraryServer.app

./releaseOSX.sh

echo "Copying to destination folder"
dest='YACReader-'$1' MacOSX-Intel'
mkdir "$dest"
cp -R ./YACReader.app "./${dest}/YACReader.app"
cp -R ./YACReaderLibrary.app "./${dest}/YACReaderLibrary.app"
cp -R ./YACReaderLibraryServer.app "./${dest}/YACReaderLibraryServer"

cp ./COPYING.txt "./${dest}/"
cp ./README.txt "./${dest}/"

#mkdir "./${dest}/icons/"
#cp ./images/db.png "./${dest}/icons/"
#cp ./images/coversPackage.png "./${dest}/icons/"

echo "Creating dmg package"
#tar -czf "${dest}".tar.gz "${dest}"
#hdiutil create "${dest}".dmg -srcfolder "./${dest}" -ov
./create-dmg --volname 'YACReader '$1' Installer' --volicon icon.icns --window-size 600 403 --icon-size 128 --app-drop-link 485 90 --background background.png --icon YACReader 80 90 --icon YACReaderLibrary 235 90 --eula COPYING.txt --icon YACReaderLibraryServer 470 295 --icon README.txt 120 295 --icon COPYING.txt 290 295 "./${dest}.dmg" "./${dest}"

echo "Done!"
