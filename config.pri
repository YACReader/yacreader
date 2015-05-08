#functions to automatically initialize some of YACReader's build options to
#default values if they're not set on build time
#for a more detailed description, see INSTALL.TXT

#check Qt version
QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION, ".")
QT_VER_MAJ = $$member(QT_VERSION, 0)
QT_VER_MIN = $$member(QT_VERSION, 1)

lessThan(QT_VER_MAJ, 5) {
error(YACReader requires Qt 5 or newer but Qt $$[QT_VERSION] was detected.)
	}
lessThan(QT_VER_MIN, 5):!CONFIG(no_opengl)	{
	CONFIG += legacy_gl_widget
	message ("Qt < 5.4 detected. Using QGLWidget for coverflow.")
	}
	
#build without opengl widget support
CONFIG(no_opengl) {
	DEFINES += NO_OPENGL
}

!CONFIG(unarr):!CONFIG(7zip) {
	unix {
		!macx {
				CONFIG+=unarr
			}
		else {
				CONFIG+=7zip
			}
	
		}
	win32 {
			CONFIG+=7zip
		  }
}