#include <QApplication>
#include <QDir>
#include <QTranslator>
#include <QCommandLineParser>

#include "main_window_viewer.h"
#include "configuration.h"
#include "exit_check.h"

#include "QsLog.h"
#include "QsLogDest.h"

using namespace QsLogging;

#if defined(WIN32) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#ifdef Q_OS_MAC
#include <QEvent>
#include <QFileOpenEvent>
class YACReaderApplication : public QApplication
{
public:
    YACReaderApplication(int &argc, char **argv)
        : QApplication(argc, argv)
    {
    }

    void setWindow(MainWindowViewer *w)
    {
        window = w;
    }

protected:
    bool event(QEvent *event) override
    {
        switch (event->type()) {
        case QEvent::FileOpen:
            window->openComicFromPath(static_cast<QFileOpenEvent *>(event)->file());
            return true;
        default:
            return QApplication::event(event);
        }
    }

private:
    MainWindowViewer *window;
};
#endif

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

int main(int argc, char *argv[])
{
    qInstallMessageHandler(messageHandler);

    static const char ENV_VAR_QT_DEVICE_PIXEL_RATIO[] = "QT_DEVICE_PIXEL_RATIO";
    if (!qEnvironmentVariableIsSet(ENV_VAR_QT_DEVICE_PIXEL_RATIO) && !qEnvironmentVariableIsSet("QT_AUTO_SCREEN_SCALE_FACTOR") && !qEnvironmentVariableIsSet("QT_SCALE_FACTOR") && !qEnvironmentVariableIsSet("QT_SCREEN_SCALE_FACTORS")) {
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    }

    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

#if defined(_MSC_VER) && defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

#ifdef Q_OS_MAC
    YACReaderApplication app(argc, argv);
#else
    QApplication app(argc, argv);
#endif

#ifdef FORCE_ANGLE
    app.setAttribute(Qt::AA_UseOpenGLES);
#endif

    app.setApplicationName("YACReader");
    app.setOrganizationName("YACReader");

    if (QIcon::hasThemeIcon("YACReader")) {
        app.setWindowIcon(QIcon::fromTheme("YACReader"));
    }

    // simple commandline parser
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({ "loglevel", "Set log level. Valid values: trace, info, debug, warn, error.", "loglevel", "warning" });
    parser.addPositionalArgument("[File|Directory]", "File or directory to open.");
    QCommandLineOption comicId("comicId", "", "comicId");
    QCommandLineOption libraryId("libraryId", "", "libraryId");
    QCommandLineOption readingListId("readingListId", "", "readingListId");
// hide comicId and libraryId from help
#if QT_VERSION >= 0x050800
    comicId.setFlags(QCommandLineOption::HiddenFromHelp);
    libraryId.setFlags(QCommandLineOption::HiddenFromHelp);
    readingListId.setFlags(QCommandLineOption::HiddenFromHelp);
#else
    comicId.setHidden(true);
    libraryId.setHidden(true);
    readingListId.setHidden(true);
#endif

    // process
    parser.addOption(comicId);
    parser.addOption(libraryId);
    parser.addOption(readingListId);
    parser.process(app);

    QString destLog = YACReader::getSettingsPath() + "/yacreader.log";
    QDir().mkpath(YACReader::getSettingsPath());

    Logger &logger = Logger::instance();
    logger.setLoggingLevel(QsLogging::InfoLevel);

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
#if defined Q_OS_UNIX && !defined Q_OS_MAC
    translator.load(QString(DATADIR) + "/yacreader/languages/yacreader_" + sufix);
#else
    translator.load(QCoreApplication::applicationDirPath() + "/languages/yacreader_" + sufix);
#endif
    app.installTranslator(&translator);
    auto mwv = new MainWindowViewer();

    // some arguments need to be parsed after MainWindowViewer creation
    QStringList arglist = parser.positionalArguments();
    if (parser.isSet(comicId) && parser.isSet(libraryId) && arglist.count() >= 1) {
        OpenComicSource source;

        if (parser.isSet(readingListId)) {
            source = OpenComicSource { OpenComicSource::ReadingList, parser.value(readingListId).toULongLong() };
        } else {
            source = OpenComicSource { OpenComicSource::Folder, 33 }; // Folder is not needed to get the comic information, the comid id points to a unique comic
        }

        mwv->open(arglist.at(0), parser.value(comicId).toULongLong(), parser.value(libraryId).toULongLong(), source);
    } else if (arglist.count() >= 1) {
        mwv->openComicFromPath(arglist.at(0));
    }

#ifdef Q_OS_MAC
    app.setWindow(mwv);
#endif
    mwv->show();
    int ret = app.exec();
    delete mwv;

    // Configuration::getConfiguration().save();
    YACReader::exitCheck(ret);
#ifdef Q_OS_WIN
    logger.shutDownLoggerThread();
#endif
    return ret;
}
