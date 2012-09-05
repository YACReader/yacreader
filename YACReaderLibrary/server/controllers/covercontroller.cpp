#include "covercontroller.h"
#include "library_window.h"  //get libraries

#include "template.h"
#include "../static.h"

extern LibraryWindow * mw;

CoverController::CoverController() {}

void CoverController::service(HttpRequest& request, HttpResponse& response)
{
	response.setHeader("Content-Type", "image/jpeg");

	//response.setHeader("Content-Type", "plain/text; charset=ISO-8859-1");

	QMap<QString,QString> libraries = mw->getLibraries();

	QString path = request.getPath();
	QStringList pathElements = path.split('/');
	QString libraryName = pathElements.at(2);
	QString fileName = pathElements.at(4);

	//response.writeText(path+"<br/>");
	//response.writeText(libraryName+"<br/>");
	//response.writeText(libraries.value(libraryName)+"/.yacreaderlibrary/covers/"+fileName+"<br/>");

	QFile file(libraries.value(libraryName)+"/.yacreaderlibrary/covers/"+fileName);
	if (file.exists()) {
		if (file.open(QIODevice::ReadOnly))
		{
			qDebug("StaticFileController: Open file %s",qPrintable(file.fileName()));
			// Return the file content, do not store in cache
			while (!file.atEnd() && !file.error()) {
				response.write(file.read(65536));
			}
		}

		file.close();
	}
}