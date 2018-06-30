#! /bin/bash
#Script to create a source tarball for YACReader distribution and packaging
#By default this will use the latest git tag for the current branch or whatever
#version supplied as argument
#This should be run from YACReader's top source directory

YACVERSION=${1:-`git describe --long --tags`}
if [ ! -f "yacreader-${YACVERSION}-src.tar.xz" ]
then
	echo "Building source tarball for ${YACVERSION}"
else
	echo "Updating source tarball for ${YACVERSION}"
	rm yacreader-${YACVERSION}-src.tar* #delete old tarball, since tar can't update compressed archives
fi

git archive --format=tar --prefix=yacreader-${YACVERSION}/ HEAD | xz -c > yacreader-${YACVERSION}-src.tar.xz

#Calculate checksum to enable packagers to verify whether they are using the original tarball.
md5sum yacreader-${YACVERSION}-src.tar.xz > yacreader-${YACVERSION}-src.tar.xz.md5sum
