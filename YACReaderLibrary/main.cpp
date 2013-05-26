#include "library_window.h"

#include <QApplication>
#include <QTranslator>
#include <QSettings>
#include <QLocale>

#include "yacreader_global.h"
#include "startup.h"
#include "yacreader_local_server.h"
#define PICTUREFLOW_QT4 1

//interfaz al servidor
Startup * s;

int main( int argc, char ** argv )
{
  QApplication app( argc, argv );

  QTranslator translator;
  QString sufix = QLocale::system().name();
  translator.load(QCoreApplication::applicationDirPath()+"/languages/yacreaderlibrary_"+sufix);
  app.installTranslator(&translator);
  app.setApplicationName("YACReaderLibrary");

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

  //connections to localServer

  mw->show();
  /*mw->resize(800,480);
  mw->showMaximized();*/

  return app.exec();
}
