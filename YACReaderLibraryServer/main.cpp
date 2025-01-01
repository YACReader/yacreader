#include <QCoreApplication>
#include <QSysInfo>
#include <QDir>
#include <QCommandLineParser>
#include <QImageReader>

#include "comic_db.h"
#include "db_helper.h"
#include "yacreader_http_server.h"
#include "yacreader_global.h"
#include "yacreader_libraries.h"
#include "yacreader_local_server.h"
#include "global_info_provider.h"

#include "libraries_update_coordinator.h"

#include "libraries_updater.h"

#include "console_ui_library_creator.h"

#include <iostream>

#include "QsLog.h"
#include "QsLogDest.h"
#include "qrcodegen.hpp"
#include "ip_config_helper.h"

using namespace QsLogging;

int start(QCoreApplication &app, QCommandLineParser &parser, const QStringList &args, QSettings *settings, QTextStream &qout);
int createLibrary(QCoreApplication &app, QCommandLineParser &parser, QSettings *settings);
int updateLibrary(QCoreApplication &app, QCommandLineParser &parser, QSettings *settings);
int addLibrary(QCoreApplication &app, QCommandLineParser &parser, QSettings *settings);
int removeLibrary(QCoreApplication &app, QCommandLineParser &parser, QSettings *settings);
int listLibraries(QCoreApplication &app, QCommandLineParser &parser, QTextStream &qout);
int setPort(QCoreApplication &app, QCommandLineParser &parser, QTextStream &qout);
int rescanXmlInfo(QCoreApplication &app, QCommandLineParser &parser, QSettings *settings);

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
void printServerInfo(YACReaderHttpServer *httpServer);
void logSystemAndConfig();

// -----------------------------------------------------------------------------
// main-------------------------------------------------------------------------
// -----------------------------------------------------------------------------
int main(int argc, char **argv)
{
    qInstallMessageHandler(messageHandler);

    QCoreApplication app(argc, argv);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QImageReader::setAllocationLimit(0);
#endif

    app.setApplicationName("YACReaderLibrary");
    app.setOrganizationName("YACReader");

    QString buildNumber = ".0";

#ifdef BUILD_NUMBER
    buildNumber = QString(".%1").arg(BUILD_NUMBER);
#endif

    app.setApplicationVersion(VERSION + buildNumber);

    QTextStream qout(stdout);

    // general help
    QTranslator translator;
    QString sufix = QLocale::system().name();

#if defined Q_OS_UNIX && !defined Q_OS_MACOS
    translator.load(QString(DATADIR) + "/yacreader/languages/yacreaderlibrary_" + sufix);
#else
    translator.load(QCoreApplication::applicationDirPath() + "/languages/yacreaderlibrary_" + sufix);
#endif
    app.installTranslator(&translator);

    auto settingsPath = YACReader::getSettingsPath() + "/" + QCoreApplication::applicationName() + ".ini";

    QCommandLineParser parser;
    parser.setApplicationDescription(QString(QCoreApplication::tr("\nYACReaderLibraryServer is the headless (no gui) version of YACReaderLibrary.\n\n"
                                                                  "This appplication support persisten settings, to set them up edit this file %1\n"
                                                                  "To learn about the available settings please check the documentation at https://raw.githubusercontent.com/YACReader/yacreader/develop/YACReaderLibraryServer/SETTINGS_README.md"))
                                             .arg(settingsPath));
    parser.addHelpOption();
    const QCommandLineOption versionOption = parser.addVersionOption();
    parser.addPositionalArgument("command", "The command to execute. [start, create-library, update-library, add-library, remove-library, list-libraries, set-port, rescan-xml-info]");
    parser.addOption({ "loglevel", "Set log level. Valid values: trace, info, debug, warn, error.", "loglevel", "info" });
    parser.addOption({ "port", "Set server port (temporary). Valid values: 1-65535", "port" });
    parser.addOption({ "system-info", "Prints detailed information about the system environment, including OS version, hardware specifications, and available resources." });
    parser.parse(app.arguments());

    const QStringList args = parser.positionalArguments();
    const QString command = args.isEmpty() ? QString() : args.first();

    if (parser.isSet(versionOption)) {
        qout << "YACReaderLibraryServer"
             << " " << VERSION << Qt::endl;

        return 0;
    }

    if (parser.isSet("system-info")) {
        auto globalInfo = YACReader::getGlobalInfo();
        for (const auto &line : globalInfo.split("\n")) {
            qout << line << Qt::endl;
        }

        return 0;
    }

    QSettings *settings = new QSettings(settingsPath, QSettings::IniFormat);
    settings->beginGroup("libraryConfig");

    if (command == "start") {
        return start(app, parser, args, settings, qout);
    } else if (command == "create-library") {
        return createLibrary(app, parser, settings);
    } else if (command == "update-library") {
        return updateLibrary(app, parser, settings);
    } else if (command == "add-library") {
        return addLibrary(app, parser, settings);
    } else if (command == "remove-library") {
        return removeLibrary(app, parser, settings);
    } else if (command == "list-libraries") {
        return listLibraries(app, parser, qout);
    } else if (command == "set-port") {
        return setPort(app, parser, qout);
    } else if (command == "rescan-xml-info") {
        return rescanXmlInfo(app, parser, settings);
    } else // error
    {
        parser.process(app);
        parser.showHelp();
        return 0;
    }
}

// -----------------------------------------------------------------------------
// start------------------------------------------------------------------------
// -----------------------------------------------------------------------------
int start(QCoreApplication &app, QCommandLineParser &parser, const QStringList &args, QSettings *settings, QTextStream &qout)
{
    parser.clearPositionalArguments();
    parser.addPositionalArgument("start", "Start YACReaderLibraryServer");
    parser.process(app);

    QString destLog = YACReader::getSettingsPath() + "/yacreaderlibrary.log";
    QDir().mkpath(YACReader::getSettingsPath());

    Logger &logger = Logger::instance();

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

    DestinationPtrU fileDestination(DestinationFactory::MakeFileDestination(
            destLog, LogRotationOption::EnableLogRotation, MaxSizeBytes(1048576), MaxOldLogCount(2)));
    DestinationPtrU debugDestination(DestinationFactory::MakeDebugOutputDestination());
    logger.addDestination(std::move(debugDestination));
    logger.addDestination(std::move(fileDestination));

    QTranslator translator;
    QString sufix = QLocale::system().name();
#if defined Q_OS_UNIX && !defined Q_OS_MACOS
    translator.load(QString(DATADIR) + "/yacreader/languages/yacreaderlibrary_" + sufix);
#else
    translator.load(QCoreApplication::applicationDirPath() + "/languages/yacreaderlibrary_" + sufix);
#endif
    app.installTranslator(&translator);

    QTranslator viewerTranslator;
#if defined Q_OS_UNIX && !defined Q_OS_MACOS
    viewerTranslator.load(QString(DATADIR) + "/yacreader/languages/yacreader_" + sufix);
#else
    viewerTranslator.load(QCoreApplication::applicationDirPath() + "/languages/yacreader_" + sufix);
#endif
    app.installTranslator(&viewerTranslator);

    qRegisterMetaType<ComicDB>("ComicDB");

    QLOG_INFO() << "YACReaderLibraryServer attempting to start";

    logSystemAndConfig();

    if (YACReaderLocalServer::isRunning()) // allow one server instance
    {
        QLOG_WARN() << "another instance of YACReaderLibrary is running";
#ifdef Q_OS_WIN
        logger.shutDownLoggerThread();
#endif
        return 0;
    }

    QLOG_INFO() << "YACReaderLibrary starting";

    // server
    YACReaderHttpServer *httpServer = new YACReaderHttpServer();
    if (parser.isSet("port")) {
        bool valid;
        qint32 port = parser.value("port").toInt(&valid);
        if (!valid || port < 1 || port > 65535) {
            qout << "Error: " << parser.value("port") << " is not a valid port" << Qt::endl;
            parser.showHelp();
            return 0;
        } else {
            httpServer->start(port);
        }

    } else {
        httpServer->start();
    }

    printServerInfo(httpServer);

    // Update libraries to new versions
    LibrariesUpdater updater;
    updater.updateIfNeeded();

    YACReaderLocalServer *localServer = new YACReaderLocalServer();

    YACReaderLibraries libraries;
    auto librariesUpdateCoordinator = new LibrariesUpdateCoordinator(settings, libraries, []() {
        return true;
    });

    app.connect(librariesUpdateCoordinator, &LibrariesUpdateCoordinator::updateStarted, &app, []() {
        QLOG_INFO() << "Starting libraries update";
    });
    app.connect(librariesUpdateCoordinator, &LibrariesUpdateCoordinator::updateEnded, &app, []() {
        QLOG_INFO() << "Done updating libraries";
    });

    librariesUpdateCoordinator->init();

    int ret = app.exec();

    QLOG_INFO() << "YACReaderLibrary closed with exit code :" << ret;

    // shutdown
    httpServer->stop();
    delete httpServer;
    localServer->close();
    delete localServer;
#ifdef Q_OS_WIN
    logger.shutDownLoggerThread();
#endif
    return ret;
}

// -----------------------------------------------------------------------------
// create-library---------------------------------------------------------------
// -----------------------------------------------------------------------------
int createLibrary(QCoreApplication &app, QCommandLineParser &parser, QSettings *settings)
{
    parser.clearPositionalArguments();
    parser.addPositionalArgument("create-library", "Creates a library named \"name\" in the specified destination <path>");
    parser.addPositionalArgument("name", "Library name", "\"name\"");
    parser.addPositionalArgument("path", "Path to the folder where the library will be created", "<path>");
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.length() != 3) {
        parser.showHelp();
        return 0;
    }

    ConsoleUILibraryCreator *libraryCreatorUI = new ConsoleUILibraryCreator(settings);
    libraryCreatorUI->createLibrary(args.at(1), args.at(2));

    return 0;
}

// -----------------------------------------------------------------------------
// update-library---------------------------------------------------------------
// -----------------------------------------------------------------------------
int updateLibrary(QCoreApplication &app, QCommandLineParser &parser, QSettings *settings)
{
    parser.clearPositionalArguments();
    parser.addPositionalArgument("update-library", "Updates an existing library at <path>");
    parser.addPositionalArgument("path", "Path to the library to be updated", "<path>");
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.length() != 2) {
        parser.showHelp();
        return 0;
    }

    ConsoleUILibraryCreator *libraryCreatorUI = new ConsoleUILibraryCreator(settings);
    libraryCreatorUI->updateLibrary(args.at(1));

    return 0;
}

// -----------------------------------------------------------------------------
// add-library---------------------------------------------------------------
// -----------------------------------------------------------------------------
int addLibrary(QCoreApplication &app, QCommandLineParser &parser, QSettings *settings)
{
    parser.clearPositionalArguments();
    parser.addPositionalArgument("add-library", "Adds an exiting library named \"name\" at the specified origin <path>");
    parser.addPositionalArgument("name", "Library name", "\"name\"");
    parser.addPositionalArgument("path", "Path to the folder where the library is", "<path>");
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.length() != 3) {
        parser.showHelp();
        return 0;
    }

    ConsoleUILibraryCreator *libraryCreatorUI = new ConsoleUILibraryCreator(settings);
    libraryCreatorUI->addExistingLibrary(args.at(1), args.at(2));

    return 0;
}

// -----------------------------------------------------------------------------
// remove-library---------------------------------------------------------------
// -----------------------------------------------------------------------------
int removeLibrary(QCoreApplication &app, QCommandLineParser &parser, QSettings *settings)
{
    parser.clearPositionalArguments();
    parser.addPositionalArgument("remove-library", "Removes a library named \"name\" from the list of libraries");
    parser.addPositionalArgument("name", "Library name", "\"name\"");
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.length() != 2) {
        parser.showHelp();
        return 0;
    }

    ConsoleUILibraryCreator *libraryCreatorUI = new ConsoleUILibraryCreator(settings);
    libraryCreatorUI->removeLibrary(args.at(1));

    return 0;
}

// -----------------------------------------------------------------------------
// list-libraries---------------------------------------------------------------
// -----------------------------------------------------------------------------
int listLibraries(QCoreApplication &app, QCommandLineParser &parser, QTextStream &qout)
{
    parser.clearPositionalArguments();
    parser.addPositionalArgument("list-libraries", "List all available libraries");
    parser.process(app);

    YACReaderLibraries libraries = DBHelper::getLibraries();
    for (QString libraryName : libraries.getNames())
        qout << libraryName << " : " << libraries.getPath(libraryName) << Qt::endl;

    return 0;
}

// -----------------------------------------------------------------------------
// set-port---------------------------------------------------------------------
// -----------------------------------------------------------------------------
int setPort(QCoreApplication &app, QCommandLineParser &parser, QTextStream &qout)
{
    parser.clearPositionalArguments();
    parser.addPositionalArgument("set-port", "Set server port (persistent).");
    parser.addPositionalArgument("port", "1-65535", "<port>");
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.length() != 2) {
        parser.showHelp();
        return 0;
    }

    bool valid;
    qint32 port = args.at(1).toInt(&valid);
    if (!valid || port < 1 || port > 65535) {
        qout << "Invalid server port";
        parser.showHelp();
        return 0;
    }

    QSettings *settings = new QSettings(YACReader::getSettingsPath() + "/" + QCoreApplication::applicationName() + ".ini", QSettings::IniFormat);
    settings->setValue("listener/port", QString::number(port));
    delete settings;
    return 0;
}

// -----------------------------------------------------------------------------
// rescan-xml-info--------------------------------------------------------------
// -----------------------------------------------------------------------------
int rescanXmlInfo(QCoreApplication &app, QCommandLineParser &parser, QSettings *settings)
{
    parser.clearPositionalArguments();
    parser.addPositionalArgument("rescan-xml-info", "Rescans all the files from an existing library at <path> and imports any missing tags from legacy ComicInfo.xml.");
    parser.addPositionalArgument("path", "Path to the library to scan", "<path>");
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.length() != 2) {
        parser.showHelp();
        return 0;
    }

    ConsoleUILibraryCreator *libraryCreatorUI = new ConsoleUILibraryCreator(settings);
    libraryCreatorUI->rescanXMLInfoLibrary(args.at(1));

    return 0;
}

// -----------------------------------------------------------------------------

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

void logSystemAndConfig()
{
    QLOG_INFO() << "---------- System & configuration ----------";
    auto globalInfo = YACReader::getGlobalInfo();
    for (const auto &line : globalInfo.split("\n")) {
        QLOG_INFO() << line;
    }
    QLOG_INFO() << "Libraries: " << DBHelper::getLibraries().getLibraries();
    QLOG_INFO() << "--------------------------------------------";
}

void printServerInfo(YACReaderHttpServer *httpServer)
{
    auto addresses = getIpAddresses();
    QLOG_INFO() << "Running on" << addresses.first() + ":" + httpServer->getPort().toLocal8Bit() << "\n";

    qrcodegen::QrCode code = qrcodegen::QrCode::encodeText(
            (addresses.first() + ":" + httpServer->getPort()).toLocal8Bit(),
            qrcodegen::QrCode::Ecc::LOW);
    int border = 4;
    for (int y = -border; y < code.getSize() + border; y += 2) {
        QString QRCodeString;
        for (int x = -border - 1; x < code.getSize() + border + 1; x++) {
            QRCodeString.append((code.getModule(x, y) && code.getModule(x, y + 1))
                                        ? " "
                                        : code.getModule(x, y + 1) ? u8"\u2580"
                                        : code.getModule(x, y)     ? u8"\u2584"
                                                                   : u8"\u2588");
        }
        QLOG_INFO() << QRCodeString;
    }
    if (addresses.length() > 1) {
        QLOG_INFO() << addresses.length() - 1 << "more network interfaces detected";
    }
}
