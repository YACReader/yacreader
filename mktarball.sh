#! /bin/bash

YACVERSION=7.2.0

if [ ! -f "yacreader-${YACVERSION}-src.tar.xz" ]
then
	echo "Building source tarball"
	tar cfJ yacreader-${YACVERSION}-src.tar.xz --exclude='.*' --exclude 'dependencies' --exclude 'yacreader*tar*' *
	md5sum yacreader-${YACVERSION}-src.tar.xz > yacreader-${YACVERSION}-src.tar.xz.md5sum
else
	echo "Updating source tarball"
	rm yacreader-${YACVERSION}-src.tar* #delete old tarball, since tar can't update compressed archives
	tar cfJ yacreader-${YACVERSION}-src.tar.xz --exclude='.*' --exclude 'dependencies' --exclude 'yacreader*tar*' *
	md5sum yacreader-${YACVERSION}-src.tar.xz > yacreader-${YACVERSION}-src.tar.xz.md5sum
fi
