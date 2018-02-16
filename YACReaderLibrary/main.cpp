#include "library_window.h"

#include <QApplication>
#include <QTranslator>
#include <QSettings>
#include <QLocale>
#include <QDir>
#include <QSysInfo>
#include <QFileInfo>
#ifndef use_unarr
  #include <QLibrary>
#endif
#include <QCommandLineParser>

#include "yacreader_global.h"
#include "startup.h"
#include "yacreader_local_server.h"
#include "comic_db.h"
#include "db_helper.h"
#include "yacreader_libraries.h"
#include "exit_check.h"
#include "opengl_checker.h"

#include "QsLog.h"
#include "QsLogDest.h"

#define PICTUREFLOW_QT4 1

//interfaz al servidor
Startup * s;

using namespace QsLogging;

void logSystemAndConfig()
{
  QLOG_INFO() << "---------- System & configuration ----------";
  QLOG_INFO() << "OS:" << QSysInfo::prettyProductName() << "Version: " << QSysInfo::productVersion();
  QLOG_INFO() << "Kernel:" << QSysInfo::kernelType() << QSysInfo::kernelVersion() << "Architecture:" << QSysInfo::currentCpuArchitecture();

#ifndef use_unarr
  #ifdef Q_OS_WIN
    if(QLibrary::isLibrary(QApplication::applicationDirPath()+"/utils/7z.dll"))
  #elif defined Q_OS_UNIX && !defined Q_OS_MAC
    if(QLibrary::isLibrary(QString(LIBDIR)+"/yacreader/7z.so") | QLibrary::isLibrary(QString(LIBDIR)+"/p7zip/7z.so"))
  #else
    if(QLibrary::isLibrary(QApplication::applicationDirPath()+"/utils/7z.so"))
  #endif
    QLOG_INFO() << "7z : found";
    else
    QLOG_ERROR() << "7z : not found";
#else // use_unarr
	QLOG_INFO() << "using unarr decompression backend";
#endif // use_unarr

#if defined Q_OS_UNIX && !defined Q_OS_MAC
    if(QFileInfo(QString(BINDIR)+"/qrencode").exists())
#else
    if(QFileInfo(QApplication::applicationDirPath()+"/utils/qrencode.exe").exists() || QFileInfo("./util/qrencode").exists())
#endif
        QLOG_INFO() << "qrencode : found";
    else
        QLOG_INFO() << "qrencode : not found";

	QSettings settings(YACReader::getSettingsPath()+"/YACReaderLibrary.ini",QSettings::IniFormat);
    settings.beginGroup("libraryConfig");
	if(settings.value(SERVER_ON,true).toBool())
        QLOG_INFO() << "server : enabled";
    else
        QLOG_INFO() << "server : disabled";

    if(settings.value(USE_OPEN_GL).toBool())
        QLOG_INFO() << "OpenGL : enabled"  << " - " << (settings.value(V_SYNC).toBool()?"VSync on":"VSync off");
    else
        QLOG_INFO() << "OpenGL : disabled";

    OpenGLChecker checker;
    QLOG_INFO() << "OpenGL version : " << checker.textVersionDescription();

	  QLOG_INFO() << "Libraries: " << DBHelper::getLibraries().getLibraries();
    QLOG_INFO() << "--------------------------------------------";
}

int main( int argc, char ** argv )
{
  QApplication app( argc, argv );

#ifdef FORCE_ANGLE
  app.setAttribute(Qt::AA_UseOpenGLES);
#endif

  app.setApplicationName("YACReaderLibrary");
  app.setOrganizationName("YACReader");
  app.setApplicationVersion(VERSION);
  app.setAttribute(Qt::AA_UseHighDpiPixmaps);

  if (QIcon::hasThemeIcon("YACReaderLibrary")) {
    app.setWindowIcon(QIcon::fromTheme("YACReaderLibrary"));
  }

  QString destLog = YACReader::getSettingsPath()+"/yacreaderlibrary.log";
  QDir().mkpath(YACReader::getSettingsPath());

  Logger& logger = Logger::instance();
  logger.setLoggingLevel(QsLogging::InfoLevel);

  DestinationPtr fileDestination(DestinationFactory::MakeFileDestination(
    destLog, EnableLogRotation, MaxSizeBytes(1048576), MaxOldLogCount(2)));
  DestinationPtr debugDestination(DestinationFactory::MakeDebugOutputDestination());
  logger.addDestination(debugDestination);
  logger.addDestination(fileDestination);

  QTranslator translator;
  QString sufix = QLocale::system().name();
#if defined Q_OS_UNIX && !defined Q_OS_MAC
  translator.load(QString(DATADIR)+"/yacreader/languages/yacreaderlibrary_"+sufix);
#else
  translator.load(QCoreApplication::applicationDirPath()+"/languages/yacreaderlibrary_"+sufix);
#endif
  app.installTranslator(&translator);

  QTranslator viewerTranslator;
#if defined Q_OS_UNIX && !defined Q_OS_MAC
  viewerTranslator.load(QString(DATADIR)+"/yacreader/languages/yacreader_"+sufix);
#else
  viewerTranslator.load(QCoreApplication::applicationDirPath()+"/languages/yacreader_"+sufix);
#endif
  app.installTranslator(&viewerTranslator);

  qRegisterMetaType<ComicDB>("ComicDB");

QCommandLineParser parser;
parser.addHelpOption();
parser.addVersionOption();
parser.process(app);

#ifdef SERVER_RELEASE
  QSettings * settings = new QSettings(YACReader::getSettingsPath()+"/YACReaderLibrary.ini",QSettings::IniFormat); //TODO unificar la creaci�n del fichero de config con el servidor
  settings->beginGroup("libraryConfig");

  s = new Startup();

  if(settings->value(SERVER_ON,true).toBool())
  {
	  s->start();
  }
#endif
  QLOG_INFO() << "YACReaderLibrary attempting to start";

  logSystemAndConfig();

  if(YACReaderLocalServer::isRunning()) //s�lo se permite una instancia de YACReaderLibrary
  {
	  QLOG_WARN() << "another instance of YACReaderLibrary is running";
	  QsLogging::Logger::destroyInstance();
	  return 0;
  }
  QLOG_INFO() << "YACReaderLibrary starting";

  YACReaderLocalServer * localServer = new YACReaderLocalServer();

  LibraryWindow * mw = new LibraryWindow();

  mw->connect(localServer,SIGNAL(comicUpdated(quint64, const ComicDB &)),mw,SLOT(updateComicsView(quint64, const ComicDB &)), Qt::QueuedConnection);

  //connections to localServer

  mw->show();

  int ret = app.exec();

  QLOG_INFO() << "YACReaderLibrary closed with exit code :" << ret;

  YACReader::exitCheck(ret);

  //shutdown
  s->stop();
  delete s;
  localServer->close();
  delete localServer;
  delete mw;

  QsLogging::Logger::destroyInstance();

  return ret;
}
