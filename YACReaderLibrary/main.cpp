#include "library_window.h"

#include <QApplication>
#include "startup.h"
#define PICTUREFLOW_QT4 1

//interfaz al servidor
Startup * s;

int main( int argc, char ** argv )
{
  QApplication app( argc, argv );

  QTranslator translator;
  QString sufix = QLocale::system().name();
  translator.load(":/yacreaderlibrary_"+sufix);
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

  LibraryWindow * mw = new LibraryWindow();
  mw->resize(800,480);
  mw->showMaximized();

  return app.exec();
}
