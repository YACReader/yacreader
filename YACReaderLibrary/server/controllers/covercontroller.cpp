#include "covercontroller.h"
#include "library_window.h"  //get libraries

#include "template.h"
#include "../static.h"

extern LibraryWindow * mw;

CoverController::CoverController() {}

void CoverController::service(HttpRequest& request, HttpResponse& response)
{

	HttpSession session=Static::sessionStore->getSession(request,response,false);

	response.setHeader("Content-Type", "image/jpeg");
	response.setHeader("Connection","close");
	//response.setHeader("Content-Type", "plain/text; charset=ISO-8859-1");

	QMap<QString,QString> libraries = mw->getLibraries();

	QString path = QUrl::fromPercentEncoding(request.getPath()).toLatin1();
	QStringList pathElements = path.split('/');
	QString libraryName = pathElements.at(2);
	QString fileName = pathElements.at(4);

	//response.writeText(path+"<br/>");
	//response.writeText(libraryName+"<br/>");
	//response.writeText(libraries.value(libraryName)+"/.yacreaderlibrary/covers/"+fileName+"<br/>");

	//QFile file(libraries.value(libraryName)+"/.yacreaderlibrary/covers/"+fileName);
	//if (file.exists()) {
	//	if (file.open(QIODevice::ReadOnly))
	//	{
	//		qDebug("StaticFileController: Open file %s",qPrintable(file.fileName()));
	//		// Return the file content, do not store in cache
	//		while (!file.atEnd() && !file.error()) {
	//			response.write(file.read(131072));
	//		}
	//	}

	//	file.close();
	//}

	QImage img(libraries.value(libraryName)+"/.yacreaderlibrary/covers/"+fileName);
	if (!img.isNull()) {

		int width = 80;
		if(session.getDisplayType()=="retina")
			width = 160;
		img = img.scaledToWidth(width,Qt::SmoothTransformation);
		QByteArray ba;
		QBuffer buffer(&ba);
		buffer.open(QIODevice::WriteOnly);
		img.save(&buffer, "JPG");
		response.write(ba,true);
	}
}

