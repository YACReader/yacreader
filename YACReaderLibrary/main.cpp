#include "library_window.h"

#include <QApplication>
#include <QTranslator>
#include <QSettings>
#include <QLocale>
#include <QDir>
#include <QSysInfo>
#include <QFileInfo>
#include <QSettings>
#include <QLibrary>

#if QT_VERSION >= 0x050000
	#include <QStandardPaths>
#else
	#include <QDesktopServices>
#endif

#include "yacreader_global.h"
#include "startup.h"
#include "yacreader_local_server.h"
#include "comic_db.h"
#include "db_helper.h"

#include "QsLog.h"
#include "QsLogDest.h"

#define PICTUREFLOW_QT4 1

//interfaz al servidor
Startup * s;

using namespace QsLogging;

void logSystemAndConfig()
{
    QLOG_INFO() << "---------- System & configuration ----------";
#if defined(Q_OS_WIN)
    switch (QSysInfo::windowsVersion())
    {
        case QSysInfo::WV_NT:
            QLOG_INFO() << "SO : Windows NT";
            break;
        case QSysInfo::WV_2000:
            QLOG_INFO() << "SO : Windows 2000";
            break;
        case QSysInfo::WV_XP:
            QLOG_INFO() << "SO : Windows XP";
            break;
        case QSysInfo::WV_2003:
            QLOG_INFO() << "SO : Windows 2003";
            break;
        case QSysInfo::WV_VISTA:
            QLOG_INFO() << "SO : Windows Vista";
            break;
        case QSysInfo::WV_WINDOWS7:
            QLOG_INFO() << "SO : Windows 7";
            break;
        case QSysInfo::WV_WINDOWS8:
            QLOG_INFO() << "SO : Windows 8";
            break;
        default:
            QLOG_INFO() << "Windows (unknown version)";
        break;
    }

#elif defined(Q_OS_MAC)

    QSysInfo::MV_LION
    QSysInfo::MV_MOUNTAINLION
    QSysInfo::MV_MAVERICKS

    switch (QSysInfo::macVersion())
    {
        case QSysInfo::MV_SNOWLEOPARD:
            QLOG_INFO() << "SO : MacOSX Snow Leopard";
            break;
        case QSysInfo::MV_LION:
            QLOG_INFO() << "SO : MacOSX Lion";
            break;
        case QSysInfo::MV_MOUNTAINLION:
            QLOG_INFO() << "SO : MacOSX Mountain Lion";
            break;
        case QSysInfo::MV_MAVERICKS:
            QLOG_INFO() << "SO : MacOSX Maverics";
            break;
        default:
            QLOG_INFO() << "SO : MacOSX (unknown version)";
        break;
    }

#elif defined(Q_OS_LINUX)
    QLOG_INFO() << "SO : Linux (unknown version)";

#else
    QLOG_INFO() << "SO : Unknown";
#endif

#ifdef Q_OS_WIN
    if(QLibrary::isLibrary(QApplication::applicationDirPath()+"/utils/7z.dll"))
#else
    if(QLibrary::isLibrary(QApplication::applicationDirPath()+"/utils/7z.so"))
#endif
        QLOG_INFO() << "7z : found";
    else
        QLOG_ERROR() << "7z : not found";

    if(QFileInfo(QApplication::applicationDirPath()+"/utils/qrencode.exe").exists() || QFileInfo("./util/qrencode").exists())
        QLOG_INFO() << "qrencode : found";
    else
        QLOG_INFO() << "qrencode : not found";

    QSettings settings(QCoreApplication::applicationDirPath()+"/YACReaderLibrary.ini",QSettings::IniFormat);
    settings.beginGroup("libraryConfig");
    if(settings.value(SERVER_ON).toBool())
        QLOG_INFO() << "server : enabled";
    else
        QLOG_INFO() << "server : disabled";

    if(settings.value(USE_OPEN_GL).toBool())
        QLOG_INFO() << "OpenGL : enabled"  << " - " << (settings.value(V_SYNC).toBool()?"VSync on":"VSync off");
    else
        QLOG_INFO() << "OpenGL : disabled";

    QLOG_INFO() << "Libraries: " << DBHelper::getLibraries();
    QLOG_INFO() << "--------------------------------------------";
}

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
    destLog, EnableLogRotation, MaxSizeBytes(1048576), MaxOldLogCount(2)));
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
  QSettings * settings = new QSettings(QCoreApplication::applicationDirPath()+"/YACReaderLibrary.ini",QSettings::IniFormat); //TODO unificar la creaci�n del fichero de config con el servidor
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
