TEMPLATE = subdirs
SUBDIRS = YACReader YACReaderLibrary YACReaderLibraryServer
YACReaderLibrary.depends = YACReader
!CONFIG(no_tests): SUBDIRS += tests
