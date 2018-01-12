#! /bin/bash
#Script to create a source tarball for YACReader distribution and packaging
#This should be run from YACReaders top source directory

YACVERSION=9.0.0-rc1
if [ ! -f "yacreader-${YACVERSION}-src.tar.xz" ]
then
	echo "Building source tarball"
else
	echo "Updating source tarball"
	rm yacreader-${YACVERSION}-src.tar* #delete old tarball, since tar can't update compressed archives
fi

hg archive -t tar -p yacreader-${YACVERSION} -X "dependencies/{unarr,pdfium,poppler}"\
  yacreader-${YACVERSION}-src.tar
xz -c yacreader-${YACVERSION}-src.tar > yacreader-${YACVERSION}-src.tar.xz
rm yacreader-${YACVERSION}-src.tar

#Calculate checksum to enable packagers to verify whether they are using the original tarball.
md5sum yacreader-${YACVERSION}-src.tar.xz > yacreader-${YACVERSION}-src.tar.xz.md5sum
