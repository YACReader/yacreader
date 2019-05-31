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
    bool event(QEvent *event)
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

int main(int argc, char *argv[])
{

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
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    if (QIcon::hasThemeIcon("YACReader")) {
        app.setWindowIcon(QIcon::fromTheme("YACReader"));
    }

    // simple commandline parser
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("[File|Directory]", "File or directory to open.");
    QCommandLineOption comicId("comicId", "", "comicId");
    QCommandLineOption libraryId("libraryId", "", "libraryId");
// hide comicId and libraryId from help
#if QT_VERSION >= 0x050800
    comicId.setFlags(QCommandLineOption::HiddenFromHelp);
    libraryId.setFlags(QCommandLineOption::HiddenFromHelp);
#else
    comicId.setHidden(true);
    libraryId.setHidden(true);
#endif

    // process
    parser.addOption(comicId);
    parser.addOption(libraryId);
    parser.process(app);

    QString destLog = YACReader::getSettingsPath() + "/yacreader.log";
    QDir().mkpath(YACReader::getSettingsPath());

    Logger &logger = Logger::instance();
    logger.setLoggingLevel(QsLogging::InfoLevel);

    DestinationPtr fileDestination(DestinationFactory::MakeFileDestination(
            destLog, EnableLogRotation, MaxSizeBytes(1048576), MaxOldLogCount(2)));
    DestinationPtr debugDestination(DestinationFactory::MakeDebugOutputDestination());
    logger.addDestination(debugDestination);
    logger.addDestination(fileDestination);

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
        mwv->open(arglist.at(0), parser.value(comicId).toULongLong(), parser.value(libraryId).toULongLong());
    } else if (arglist.count() >= 1) {
        mwv->openComicFromPath(arglist.at(0));
    }

#ifdef Q_OS_MAC
    app.setWindow(mwv);
#endif
    mwv->show();
    int ret = app.exec();
    delete mwv;

    //Configuration::getConfiguration().save();
    YACReader::exitCheck(ret);
    return ret;
}
