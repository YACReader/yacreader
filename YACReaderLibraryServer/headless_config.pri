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
lessThan(QT_VER_MIN, 3){
	error ("You need at least Qt 5.3 to build YACReader or YACReaderLibrary")
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

unix { 
	!macx {
		packagesExist(Qt5Core) {
				message("Found QtCore")
			}
			else: {
				message("Missing dependency: QtCore")
			}
		packagesExist(Qt5Gui) {
				message("Found QtGui")
			}
			else: {
				message("Missing dependency: QtGui")
			}
		packagesExist(poppler-qt5) {
				message("Found poppler-qt5")
			}
			else: {
				message("Missing dependency: poppler-qt5")
			}
		packagesExist(Qt5Network) {
				message("Found QtNetwork")
			}
			else: {
				message("Missing dependency: QtNetwork")
			}
		packagesExist(Qt5Sql) {
				message("Found QtSql")
			}
			else: {
				message("Missing dependency: QtSql")
			}
		packagesExist(sqlite3) {
				message("Found sqlite3")
			}
			else: {
				message("Missing dependency: sqlite3")
			}
		}
}