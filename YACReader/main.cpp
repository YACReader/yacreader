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

	app.setApplicationName("YACReader");
	app.setOrganizationName("YACReader");

	QString destLog = YACReader::getSettingsPath()+"/yacreader.log";
	QDir().mkpath(YACReader::getSettingsPath());

	Logger& logger = Logger::instance();
	logger.setLoggingLevel(QsLogging::TraceLevel);

	DestinationPtr fileDestination(DestinationFactory::MakeFileDestination(
	  destLog, EnableLogRotation, MaxSizeBytes(1048576), MaxOldLogCount(2)));
	DestinationPtr debugDestination(DestinationFactory::MakeDebugOutputDestination());
	logger.addDestination(debugDestination);
	logger.addDestination(fileDestination);

	QTranslator translator;
	QString sufix = QLocale::system().name();
#if defined Q_OS_UNIX && !defined Q_OS_MAC	
	translator.load(QString(DATADIR)+"/YACReader/languages/yacreader_"+sufix);
#else
	translator.load(QCoreApplication::applicationDirPath()+"/languages/yacreader_"+sufix);
#endif	
	app.installTranslator(&translator);

	MainWindowViewer * mwv = new MainWindowViewer();
#ifdef Q_OS_MAC
    app.setWindow(mwv);
#endif
	mwv->show();

	int ret = app.exec();

	//Configuration::getConfiguration().save();
	
	YACReader::exitCheck(ret);

	return ret;
}
