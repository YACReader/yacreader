#include "library_window.h"

#include <QApplication>
#include <QTranslator>
#include <QSettings>
#include <QLocale>
#include <QTextStream>
#include <QtDebug>
#include <QDir>
#include <QDateTime>

#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#endif

#include "yacreader_global.h"
#include "startup.h"
#include "yacreader_local_server.h"
#include "comic_db.h"

#define PICTUREFLOW_QT4 1

#if QT_VERSION >= 0x050000
void yacreaderMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{

	QString txt;

	switch (type) {
		case QtDebugMsg:
			txt = QString("Debug: %1").arg(msg);
			break;
		case QtWarningMsg:
			txt = QString("Warning: %1").arg(msg);
			break;
		case QtCriticalMsg:
			txt = QString("Critical: %1").arg(msg);
			break;
		case QtFatalMsg:
			txt = QString("Fatal: %1").arg(msg);
			break;
	}

	QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));

	QFile outFile(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)+"/yacreaderlibrary.log");

	outFile.open(QIODevice::WriteOnly | QIODevice::Append);
	QTextStream ts(&outFile);
	ts << QDateTime::currentDateTime().toString() << " - " << txt << endl;
}
#else
void yacreaderMessageHandler(QtMsgType type, const char *)
{

}
#endif


//interfaz al servidor
Startup * s;

int main( int argc, char ** argv )
{
  QApplication app( argc, argv );

#if QT_VERSION >= 0x050000
  qInstallMessageHandler(yacreaderMessageHandler);
#else
  qInstallMsgHandler(yacreaderMessageHandler);
#endif

  qDebug() << "YACReaderLibrary started" << endl;

  QTranslator translator;
  QString sufix = QLocale::system().name();
  translator.load(QCoreApplication::applicationDirPath()+"/languages/yacreaderlibrary_"+sufix);
  app.installTranslator(&translator);
  
  QTranslator viewerTranslator;
  viewerTranslator.load(QCoreApplication::applicationDirPath()+"/languages/yacreader_"+sufix);
  app.installTranslator(&viewerTranslator);
  app.setApplicationName("YACReaderLibrary");

  qRegisterMetaType<ComicDB>("ComicDB");

#ifdef SERVER_RELEASE
  QSettings * settings = new QSettings(QCoreApplication::applicationDirPath()+"/YACReaderLibrary.ini",QSettings::IniFormat); //TODO unificar la creación del fichero de config con el servidor
  settings->beginGroup("libraryConfig");
  
  s = new Startup();

  if(settings->value(SERVER_ON,true).toBool())
  {
	  
	  s->start();
  }
#endif
  
  if(YACReaderLocalServer::isRunning()) //sólo se permite una instancia de YACReaderLibrary
	return 0;

  YACReaderLocalServer * localServer = new YACReaderLocalServer();

  LibraryWindow * mw = new LibraryWindow();

  mw->connect(localServer,SIGNAL(comicUpdated(quint64, const ComicDB &)),mw,SLOT(updateComicsView(quint64, const ComicDB &)));

  //connections to localServer

  mw->show();
  /*mw->resize(800,480);
  mw->showMaximized();*/

  int ret = app.exec();

  //server shutdown
  s->stop();
  delete s;

  qDebug() << "YACReaderLibrary closed" << endl;

  return ret;
}
