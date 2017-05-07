#! /bin/bash
#Script to create a source tarball for YACReader distribution and packaging
#This should be run from YACReaders top source directory

YACVERSION=8.6
if [ -f Makefile ]
then
	make distclean
fi
if [ ! -f "yacreader-${YACVERSION}-src.tar.xz" ]
then
	echo "Building source tarball"
else
	echo "Updating source tarball"
	rm yacreader-${YACVERSION}-src.tar* #delete old tarball, since tar can't update compressed archives
fi
#Use tar's --exclude feature to make sure we get a pristine tar for distribution.
#Exclude all version control system related files and rename the top directory in the tarball using --transform.
tar cfJ yacreader-${YACVERSION}-src.tar.xz  --exclude '*.rej' --exclude '*.orig' --exclude '*.gch' --exclude 'dependencies' --exclude '*.o' \
--exclude 'yacreader*tar*' --exclude '.hg*' --exclude 'lib7zip' --exclude 'libp7zip' --exclude 'unarr-master' --exclude-vcs ./* --transform s/./yacreader-${YACVERSION}/
#Calculate checksum to enable packagers to verify whether they are using the original tarball. 
md5sum yacreader-${YACVERSION}-src.tar.xz > yacreader-${YACVERSION}-src.tar.xz.md5sum