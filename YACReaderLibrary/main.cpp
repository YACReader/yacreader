#include "library_window.h"

#include <QApplication>
#include "startup.h"
#define PICTUREFLOW_QT4 1

//interfaz al servidor
LibraryWindow * mw;

int main( int argc, char ** argv )
{
  QApplication app( argc, argv );

  QTranslator translator;
  QString sufix = QLocale::system().name();
  translator.load(":/yacreaderlibrary_"+sufix);
  app.installTranslator(&translator);
  app.setApplicationName("YACReaderLibrary");

  mw = new LibraryWindow();
  mw->resize(800,480);
  mw->showMaximized();

  Startup * s = new Startup();
  s->start();

  return app.exec();
}
