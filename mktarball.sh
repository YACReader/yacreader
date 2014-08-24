#! /bin/bash
#script to create a source tarball for YACReader distribution and packaging
YACVERSION=7.2.0
if [ -f Makefile ]
then
	make distclean
fi
if [ ! -f "yacreader_${YACVERSION}-src.tar.xz" ]
then
	echo "Building source tarball"
else
	echo "Updating source tarball"
	rm yacreader_${YACVERSION}-src.tar* #delete old tarball, since tar can't update compressed archives
fi
#use --exclude to make sure we get a pristine tar
#might not work out of the box with bsdtar!
tar cfJ yacreader_${YACVERSION}-src.tar.xz  --exclude '*.rej' --exclude '*.orig' --exclude '*.gch' --exclude 'dependencies' \
--exclude 'yacreader*tar*' --exclude '.hg*' --exclude 'libp7zip' --exclude 'lib7zip' ./* --transform s/./yacreader_${YACVERSION}/
md5sum yacreader_${YACVERSION}-src.tar.xz > yacreader_${YACVERSION}-src.tar.xz.md5sum