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


int main(int argc, char * argv[])
{
	QApplication app(argc, argv);

	QTranslator translator;
	QString sufix = QLocale::system().name();
	translator.load(":/yacreader_"+sufix);
	app.installTranslator(&translator);
	app.setApplicationName("YACReader");

	MainWindowViewer mwv;
	mwv.show();

	int ret = app.exec();

	Configuration::getConfiguration().save();

	return ret;
}
