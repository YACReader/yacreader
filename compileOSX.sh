#! /bin/bash
if [ $2 == "clean" ]; then
./cleanOSX.sh
fi

echo "Compiling YACReader"
cd ./YACReader
/Users/luisangel/Qt/5.3/clang_64/bin/qmake -spec macx-clang "CONFIG+=release"
#qmake -spec macx-g++ "CONFIG+=release"
make
cd ..

echo "Compiling YACReaderLibrary"
cd ./YACReaderLibrary
/Users/luisangel/Qt/5.3/clang_64/bin/qmake -spec macx-clang "CONFIG+=release"
#qmake -spec macx-g++ "CONFIG+=release"
make
cd ..

echo "Configuring release apps"

cp -R ./YACReader/YACReader.app ./YACReader.app
cp -R ./YACReaderLibrary/YACReaderLibrary.app ./YACReaderLibrary.app

./releaseOSX.sh

#cp -R ./PlugInsYACReader ./YACReader.app/Contents/PlugIns
#cp -R ./PlugInsLibrary ./YACReaderLibrary.app/Contents/PlugIns

echo "Copying to destination folder"
dest='YACReader-'$1' MacOSX-Intel'
mkdir "$dest"
cp -R ./YACReader.app "./${dest}/YACReader.app"
cp -R ./YACReaderLibrary.app "./${dest}/YACReaderLibrary.app"
cp ./COPYING.txt "./${dest}/"
cp ./README.txt "./${dest}/"

mkdir "./${dest}/icons/"
cp ./images/db.png "./${dest}/icons/"
cp ./images/coversPackage.png "./${dest}/icons/"

echo "Creating dmg package"
#tar -czf "${dest}".tar.gz "${dest}"
#hdiutil create "${dest}".dmg -srcfolder "./${dest}" -ov
./create-dmg --volname 'YACReader '$1' Installer' --volicon icon.icns --window-size 600 403 --icon-size 128 --app-drop-link 485 90 --background background.png --icon YACReader 80 90 --icon YACReaderLibrary 235 90 --eula COPYING.txt --icon icons 470 295 --icon README.txt 120 295 --icon COPYING.txt 290 295 "./${dest}.dmg" "./${dest}"

echo "Done!"
