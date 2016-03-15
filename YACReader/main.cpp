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
#include <QSysInfo>
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
	 #define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
	 #define new DEBUG_NEW
  #endif

#ifdef Q_OS_MAC
#include <QEvent>
class YACReaderApplication: public QApplication
{
    public:
        YACReaderApplication(int & argc, char ** argv) : QApplication(argc,argv)
        {}

        void setWindow(MainWindowViewer * w)
        {
            window = w;
        }

    protected:
        bool event(QEvent * event)
        {
            switch(event->type())
            {
                case QEvent::FileOpen:
                    window->openComicFromPath(static_cast<QFileOpenEvent *>(event)->file());
                    return true;
                default:
                    return QApplication::event(event);
            }
        }
    private:
        MainWindowViewer * window;
};
#endif

int main(int argc, char * argv[])
{

	#if defined(_MSC_VER) && defined(_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

//fix for misplaced text in Qt4.8 and Mavericks
#ifdef Q_OS_MAC
    #if QT_VERSION < 0x050000
        if(QSysInfo::MacintoshVersion > QSysInfo::MV_10_8)
            QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
    #endif
#endif


#ifdef Q_OS_MAC
    YACReaderApplication app(argc,argv);
#else
	QApplication app(argc, argv);
#endif

#ifdef FORCE_ANGLE
    app.setAttribute(Qt::AA_UseOpenGLES);
#endif

	app.setApplicationName("YACReader");
	app.setOrganizationName("YACReader");
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
	//simple command line parser
	//will be replaced by QCommandLineParser in the future
	QStringList optlist;
	QStringList arglist;
	
	if (argc > 1)
	{
		//extract options and arguments
		optlist = QCoreApplication::arguments().filter(QRegExp ("^-{1,2}")); 		//options starting with "-"
		arglist = QCoreApplication::arguments().filter(QRegExp ("^(?!-{1,2})")); 	//positional arguments
		//deal with standard options
		if (!optlist.isEmpty())
		{
			QTextStream parser(stdout);
            if (optlist.contains("--version") || optlist.contains("-v"))
			{
				parser << app.applicationName() << " " << QString(VERSION) << endl << "Copyright 2014 by Luis Angel San Martin Rodriguez" << endl;
				return 0;
			}
            if (optlist.contains("--help") || optlist.contains("-h"))
			{
				parser << endl << "Usage: YACReader [File|Directory|Option]" << endl << endl;
				parser << "Options:" << endl;
				parser << "  -h, --help\t\tDisplay this text and exit." << endl;
				parser << "  -v, --version\t\tDisplay version information and exit." << endl << endl;
				parser << "Arguments:" << endl;
				parser << "  file\t\t\tOpen comic file." <<endl;
				parser << "  directory\t\tOpen comic directory." << endl << endl;
				return 0;
			}
		}
	}
	
	QString destLog = YACReader::getSettingsPath()+"/yacreader.log";
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
	translator.load(QString(DATADIR)+"/yacreader/languages/yacreader_"+sufix);
#else
	translator.load(QCoreApplication::applicationDirPath()+"/languages/yacreader_"+sufix);
#endif	
	app.installTranslator(&translator);
	MainWindowViewer * mwv = new MainWindowViewer();
	//parser code for comic loading needs to be processed after MainWindowViewer creation
	//if we have a valid request, open it - if not, load normally
	if (argc > 1)
	{
		if (!optlist.filter("--comicId=").isEmpty() && !optlist.filter("--libraryId=").isEmpty())	
		{
			if (arglist.count()>1)
			{
				mwv->open(arglist.at(1), optlist.filter("--comicId=").at(0).split("=").at(1).toULongLong(), optlist.filter("--libraryId=").at(0).split("=").at(1).toULongLong());
			}
		}
		else if ((arglist.count()>1))
		{
			//open first positional argument, silently ignore all following positional arguments
			mwv->openComicFromPath(arglist.at(1));
		}
	}
#ifdef Q_OS_MAC
    app.setWindow(mwv);
#endif
	mwv->show();

	int ret = app.exec();

	//Configuration::getConfiguration().save();
	
	YACReader::exitCheck(ret);

	return ret;
}
