#include <QApplication>
#include <QPixmap>
#include <QLabel>
//#include <QMetaObject>
#include <QPushButton>
#include <QMainWindow>
#include <QtCore>
#include <QThread>
#include <QFile>
#include <QDataStream>
#include <QTranslator>
#include "main_window_viewer.h"
#include "configuration.h"



 #if defined(WIN32) && defined(_DEBUG)
	 #define _CRTDBG_MAP_ALLOC
	 #include <stdlib.h>
	 #include <crtdbg.h>
	 #define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
	 #define new DEBUG_NEW
  #endif

int main(int argc, char * argv[])
{
	#if defined(_MSC_VER) && defined(_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	QApplication app(argc, argv);

	app.setApplicationName("YACReader");
	app.setOrganizationName("YACReader");

	QTranslator translator;
	QString sufix = QLocale::system().name();
	translator.load(QCoreApplication::applicationDirPath()+"/languages/yacreader_"+sufix);
	app.installTranslator(&translator);

	MainWindowViewer * mwv = new MainWindowViewer();
	mwv->show();

	int ret = app.exec();

	Configuration::getConfiguration().save();
	
	return ret;
}
