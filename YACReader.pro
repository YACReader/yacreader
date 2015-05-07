TEMPLATE = subdirs
SUBDIRS = YACReader YACReaderLibrary
YACReaderLibrary.depends = YACReader

#check Qt version
QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION, ".")
QT_VER_MAJ = $$member(QT_VERSION, 0)
QT_VER_MIN = $$member(QT_VERSION, 1)

lessThan(QT_VER_MAJ, 5) {
error(YACReader requires Qt 5 or newer but Qt $$[QT_VERSION] was detected.)
	}
lessThan(QT_VER_MIN, 4)	{
	CONFIG += legacy_gl_widget
	message ("Qt < 5.4 detected. Using QGLWidget for coverflow.")
	}
	
unix
	{
	CONFIG+=unarr
	}