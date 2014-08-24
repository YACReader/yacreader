#! /bin/bash
#script to create a source tarball for YACReader distribution and packaging
YACVERSION=7.2.0

if [ ! -f "yacreader-${YACVERSION}-src.tar.xz" ]
then
	echo "Building source tarball"
else
	echo "Updating source tarball"
	rm yacreader-${YACVERSION}-src.tar* #delete old tarball, since tar can't update compressed archives
fi

tar cfJ yacreader-${YACVERSION}-src.tar.xz --exclude-vcs --exclude-vcs-ignores --exclude 'dependencies' \
--exclude 'yacreader*tar*' ./* --transform s/./yacreader-${YACVERSION}/
md5sum yacreader-${YACVERSION}-src.tar.xz > yacreader-${YACVERSION}-src.tar.xz.md5sum
