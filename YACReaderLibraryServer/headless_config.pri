# functions to automatically initialize some of YACReaderLibraryServer's build
# options to default values if they're not set at build time
# for a more detailed description, see INSTALL.TXT

include (../config.pri)

unix!macx {
	!contains(QT_CONFIG, no-pkg-config) {
		packagesExist(Qt5Core) {
				message("Found Qt5Core")
			}	else: {
				message("Missing dependency: Qt5Core")
			}
		packagesExist(Qt5Gui) {
				message("Found Qt5Gui")
			} else: {
				message("Missing dependency: Qt5Gui")
			}
		packagesExist(Qt5Network) {
				message("Found Qt5Network")
			} else: {
				message("Missing dependency: Qt5Network")
			}
		packagesExist(Qt5Sql) {
				message("Found Qt5Sql")
			} else: {
				message("Missing dependency: Qt5Sql")
			}
		packagesExist(sqlite3) {
				message("Found sqlite3")
			} else: {
				message("Missing dependency: sqlite3")
			}
		} else {
			message("Qmake was compiled without support for pkg-config. Skipping dependency checks.")
		}
}
