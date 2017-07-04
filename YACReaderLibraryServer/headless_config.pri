#functions to automatically initialize some of YACReader's build options to
#default values if they're not set on build time
#for a more detailed description, see INSTALL.TXT

#check Qt version
QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION, ".")
QT_VER_MAJ = $$member(QT_VERSION, 0)
QT_VER_MIN = $$member(QT_VERSION, 1)

include (../config.pri)

unix { 
	!macx {
		packagesExist(Qt5Core) {
				message("Found Qt5Core")
			}
			else: {
				message("Missing dependency: Qt5Core")
			}
		packagesExist(Qt5Gui) {
				message("Found Qt5Gui")
			}
			else: {
				message("Missing dependency: Qt5Gui")
			}
		packagesExist(poppler-qt5) {
				message("Found poppler-qt5")
			}
			else: {
				message("Missing dependency: poppler-qt5")
			}
		packagesExist(Qt5Network) {
				message("Found Qt5Network")
			}
			else: {
				message("Missing dependency: Qt5Network")
			}
		packagesExist(Qt5Sql) {
				message("Found Qt5Sql")
			}
			else: {
				message("Missing dependency: Qt5Sql")
			}
		packagesExist(sqlite3) {
				message("Found sqlite3")
			}
			else: {
				message("Missing dependency: sqlite3")
			}
		}
}
