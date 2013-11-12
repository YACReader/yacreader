#include "library_window.h"

#include <QApplication>
#include <QTranslator>
#include <QSettings>
#include <QLocale>
#include <QDir>
#if QT_VERSION >= 0x050000
	#include <QStandardPaths>
#else
	#include <QDesktopServices>
#endif

#include "yacreader_global.h"
#include "startup.h"
#include "yacreader_local_server.h"
#include "comic_db.h"

#include "QsLog.h"
#include "QsLogDest.h"

#define PICTUREFLOW_QT4 1

//interfaz al servidor
Startup * s;

using namespace QsLogging;

int main( int argc, char ** argv )
{
  QApplication app( argc, argv );

  app.setApplicationName("YACReaderLibrary");
  app.setOrganizationName("YACReader");

#if QT_VERSION >= 0x050000
  QString destLog = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)+"/yacreaderlibrary.log";
  QString destErr = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)+"/yacreaderlibrary.err";
  QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));

#else
  QString destLog = QDesktopServices::storageLocation(QDesktopServices::DataLocation)+"/yacreaderlibrary.log";
  QString destErr = QDesktopServices::storageLocation(QDesktopServices::DataLocation)+"/yacreaderlibrary.err";
  QDir().mkpath(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
#endif

  Logger& logger = Logger::instance();
  logger.setLoggingLevel(QsLogging::TraceLevel);

  DestinationPtr fileDestination(DestinationFactory::MakeFileDestination(
	destLog, EnableLogRotation, MaxSizeBytes(2048), MaxOldLogCount(2)));
  DestinationPtr debugDestination(DestinationFactory::MakeDebugOutputDestination());
  logger.addDestination(debugDestination);
  logger.addDestination(fileDestination);

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
  QLOG_INFO() << "YACReaderLibrary attempting to start";
  if(YACReaderLocalServer::isRunning()) //sólo se permite una instancia de YACReaderLibrary
  {
	  QLOG_WARN() << "another instance of YACReaderLibrary is running";
	  QsLogging::Logger::destroyInstance();
	  return 0;
  }
  QLOG_INFO() << "YACReaderLibrary starting";

  YACReaderLocalServer * localServer = new YACReaderLocalServer();

  LibraryWindow * mw = new LibraryWindow();

  mw->connect(localServer,SIGNAL(comicUpdated(quint64, const ComicDB &)),mw,SLOT(updateComicsView(quint64, const ComicDB &)));

  //connections to localServer

  mw->show();

  int ret = app.exec();

  //server shutdown
  s->stop();
  delete s;

  QLOG_INFO() << "YACReaderLibrary closed";

  QsLogging::Logger::destroyInstance();

  return ret;
}
