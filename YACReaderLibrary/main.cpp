#include "library_window.h"

#include <QApplication>
#include <QTranslator>
#include <QSettings>
#include <QLocale>
#include <QDir>
#include <QSysInfo>
#include <QFileInfo>
#if !defined use_unarr && !defined use_libarchive
#include <QLibrary>
#endif
#include <QCommandLineParser>
#include <QImageReader>

#include "yacreader_global.h"
#include "yacreader_http_server.h"
#include "yacreader_local_server.h"
#include "comic_db.h"
#include "db_helper.h"
#include "yacreader_libraries.h"
#include "exit_check.h"
#include "opengl_checker.h"
#ifdef Q_OS_MACOS
#include "trayhandler.h"
#endif

#include "QsLog.h"
#include "QsLogDest.h"

#define PICTUREFLOW_QT4 1

// Server interface
YACReaderHttpServer *httpServer;

using namespace QsLogging;

void logSystemAndConfig()
{
    QLOG_INFO() << "---------- System & configuration ----------";
    QLOG_INFO() << "YACReader version:" << VERSION;
    QLOG_INFO() << "Qt version:" << qVersion();
    QLOG_INFO() << "OS:" << QSysInfo::prettyProductName() << "Version: " << QSysInfo::productVersion();
    QLOG_INFO() << "Kernel:" << QSysInfo::kernelType() << QSysInfo::kernelVersion() << "Architecture:" << QSysInfo::currentCpuArchitecture() << "ABI:" << QSysInfo::buildAbi();

#if !defined use_unarr && !defined use_libarchive
#ifdef Q_OS_WIN
    if (QLibrary::isLibrary(QApplication::applicationDirPath() + "/utils/7z.dll"))
#elif defined Q_OS_UNIX && !defined Q_OS_MACOS
    if (QLibrary::isLibrary(QString(LIBDIR) + "/yacreader/7z.so") | QLibrary::isLibrary(QString(LIBDIR) + "/7zip/7z.so"))
#else
    if (QLibrary::isLibrary(QApplication::applicationDirPath() + "/utils/7z.so"))
#endif
        QLOG_INFO() << "7z : found";
    else
        QLOG_ERROR() << "7z : not found";
#elif defined use_libarchive
    QLOG_INFO() << "using libarchive decompression backend";
#else // use_unarr
    QLOG_INFO() << "using unarr decompression backend";
#endif // use_unarr

#if defined Q_OS_UNIX && !defined Q_OS_MACOS
    if (QFileInfo(QString(BINDIR) + "/qrencode").exists())
#else
    if (QFileInfo(QApplication::applicationDirPath() + "/utils/qrencode.exe").exists() || QFileInfo("./util/qrencode").exists())
#endif
        QLOG_INFO() << "qrencode : found";
    else
        QLOG_INFO() << "qrencode : not found";

    QSettings settings(YACReader::getSettingsPath() + "/YACReaderLibrary.ini", QSettings::IniFormat);
    settings.beginGroup("libraryConfig");
    if (settings.value(SERVER_ON, true).toBool())
        QLOG_INFO() << "server : enabled";
    else
        QLOG_INFO() << "server : disabled";

    if (settings.value(USE_OPEN_GL).toBool())
        QLOG_INFO() << "OpenGL : enabled"
                    << " - " << (settings.value(V_SYNC).toBool() ? "VSync on" : "VSync off");
    else
        QLOG_INFO() << "OpenGL : disabled";

    OpenGLChecker checker;
    QLOG_INFO() << "OpenGL version : " << checker.textVersionDescription();

    QLOG_INFO() << "Libraries: " << DBHelper::getLibraries().getLibraries();
    QLOG_INFO() << "--------------------------------------------";
}

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);

    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtInfoMsg: {
        QLOG_INFO() << localMsg.constData();
        break;
    }
    case QtDebugMsg: {
        QLOG_DEBUG() << localMsg.constData();
        break;
    }

    case QtWarningMsg: {
        QLOG_WARN() << localMsg.constData();
        break;
    }

    case QtCriticalMsg: {
        QLOG_ERROR() << localMsg.constData();
        break;
    }

    case QtFatalMsg: {
        QLOG_FATAL() << localMsg.constData();
        break;
    }
    }
}

int main(int argc, char **argv)
{
    qInstallMessageHandler(messageHandler);

    static const char ENV_VAR_QT_DEVICE_PIXEL_RATIO[] = "QT_DEVICE_PIXEL_RATIO";
    if (!qEnvironmentVariableIsSet(ENV_VAR_QT_DEVICE_PIXEL_RATIO) && !qEnvironmentVariableIsSet("QT_AUTO_SCREEN_SCALE_FACTOR") && !qEnvironmentVariableIsSet("QT_SCALE_FACTOR") && !qEnvironmentVariableIsSet("QT_SCREEN_SCALE_FACTORS")) {
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    }

    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QImageReader::setAllocationLimit(0);
#endif

    QApplication app(argc, argv);

#ifdef FORCE_ANGLE
    app.setAttribute(Qt::AA_UseOpenGLES);
#endif

    app.setApplicationName("YACReaderLibrary");
    app.setOrganizationName("YACReader");
    app.setApplicationVersion(VERSION);

    // Set window icon according to Freedesktop icon specification
    // This is mostly relevant for Linux and other Unix systems
    if (QIcon::hasThemeIcon("YACReaderLibrary")) {
        app.setWindowIcon(QIcon::fromTheme("YACReaderLibrary"));
    }
    // TODO: We might want to add a fallback icon here.

    QString destLog = YACReader::getSettingsPath() + "/yacreaderlibrary.log";
    QDir().mkpath(YACReader::getSettingsPath());

    Logger &logger = Logger::instance();
#ifdef QT_DEBUG
    logger.setLoggingLevel(QsLogging::TraceLevel);
#else
    logger.setLoggingLevel(QsLogging::ErrorLevel);
#endif

    DestinationPtrU fileDestination(DestinationFactory::MakeFileDestination(
            destLog, LogRotationOption::EnableLogRotation, MaxSizeBytes(1048576), MaxOldLogCount(2)));
    DestinationPtrU debugDestination(DestinationFactory::MakeDebugOutputDestination());
    logger.addDestination(std::move(debugDestination));
    logger.addDestination(std::move(fileDestination));

    QTranslator translator;
#if defined Q_OS_UNIX && !defined Q_OS_MACOS
    translator.load(QLocale(), "yacreaderlibrary", "_", QString(DATADIR) + "/yacreader/languages");
#else
    translator.load(QLocale(), "yacreaderlibrary", "_", "languages");
#endif
    app.installTranslator(&translator);

    /*QTranslator viewerTranslator;
    #if defined Q_OS_UNIX && !defined Q_OS_MACOS
        viewerTranslator.load(QString(DATADIR) + "/yacreader/languages/yacreader_" + sufix);
    #else
        viewerTranslator.load(QCoreApplication::applicationDirPath() + "/languages/yacreader_" + sufix);
    #endif
        app.installTranslator(&viewerTranslator);*/

    qRegisterMetaType<ComicDB>("ComicDB");

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({ "loglevel", "Set log level. Valid values: trace, info, debug, warn, error.", "loglevel", "warning" });
#ifdef Q_OS_WIN
    parser.addOption({ "opengl", "Set opengl renderer. Valid values: desktop, es, software.", "gl_renderer" });
#endif
    parser.process(app);

#ifdef Q_OS_WIN
    if (parser.isSet("opengl")) {
        QTextStream qout(stdout);
        if (parser.value("opengl") == "desktop") {
            app.setAttribute(Qt::AA_UseDesktopOpenGL);
        } else if (parser.value("opengl") == "es") {
            app.setAttribute(Qt::AA_UseOpenGLES);
        } else if (parser.value("opengl") == "software") {
            qout << "Warning! This will be slow as hell. Only use this setting for"
                    "testing or as a last resort.";
            app.setAttribute(Qt::AA_UseSoftwareOpenGL);
        } else {
            qout << "Invalid value:" << parser.value("gl_renderer");
            parser.showHelp();
        }
    }
#endif

    if (parser.isSet("loglevel")) {
        if (parser.value("loglevel") == "trace") {
            logger.setLoggingLevel(QsLogging::TraceLevel);
        } else if (parser.value("loglevel") == "info") {
            logger.setLoggingLevel(QsLogging::InfoLevel);
        } else if (parser.value("loglevel") == "debug") {
            logger.setLoggingLevel(QsLogging::DebugLevel);
        } else if (parser.value("loglevel") == "warn") {
            logger.setLoggingLevel(QsLogging::WarnLevel);
        } else if (parser.value("loglevel") == "error") {
            logger.setLoggingLevel(QsLogging::ErrorLevel);
        } else {
            parser.showHelp();
        }
    }

    QLOG_INFO() << "YACReaderLibrary attempting to start";

    logSystemAndConfig();

    if (YACReaderLocalServer::isRunning()) // only a single instance of YACReaderLibrary is allowed
    {
        QLOG_WARN() << "another instance of YACReaderLibrary is running";
#ifdef Q_OS_WIN
        logger.shutDownLoggerThread();
#endif
        return 0;
    }

    QLOG_INFO() << "YACReaderLibrary starting";

#ifdef SERVER_RELEASE
    QSettings *settings = new QSettings(YACReader::getSettingsPath() + "/YACReaderLibrary.ini", QSettings::IniFormat);
    settings->beginGroup("libraryConfig");

    httpServer = new YACReaderHttpServer();

    if (settings->value(SERVER_ON, true).toBool()) {
        httpServer->start();
    }
#endif

    auto localServer = new YACReaderLocalServer();

    auto mw = new LibraryWindow();

    mw->connect(localServer, &YACReaderLocalServer::comicUpdated, mw, &LibraryWindow::updateViewsOnComicUpdate, Qt::QueuedConnection);
    mw->connect(httpServer, &YACReaderHttpServer::comicUpdated, mw, &LibraryWindow::updateViewsOnComicUpdateWithId, Qt::QueuedConnection);
    mw->connect(httpServer, &YACReaderHttpServer::clientSync, mw, &LibraryWindow::updateViewsOnClientSync, Qt::QueuedConnection);
    // connections to localServer

    // start as tray
    if (!settings->value(START_TO_TRAY, false).toBool() || !settings->value(CLOSE_TO_TRAY, false).toBool()) {
        mw->show();
    }
#ifdef Q_OS_MACOS
    else {
        OSXHideDockIcon();
    }
#endif

    app.installEventFilter(mw);

    int ret = app.exec();

    QLOG_INFO() << "YACReaderLibrary closed with exit code :" << ret;

    YACReader::exitCheck(ret);

    // shutdown
    httpServer->stop();
    delete httpServer;
    localServer->close();
    delete localServer;
    delete mw;
#ifdef Q_OS_WIN
    logger.shutDownLoggerThread();
#endif
    return ret;
}
