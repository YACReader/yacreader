#include "covercontroller_v2.h"
#include "db_helper.h"  //get libraries
#include "yacreader_libraries.h"
#include "yacreader_http_session.h"

#include "template.h"
#include "../static.h"

CoverControllerV2::CoverControllerV2() {}

void CoverControllerV2::service(HttpRequest& request, HttpResponse& response)
{
	response.setHeader("Content-Type", "image/jpeg");

	YACReaderLibraries libraries = DBHelper::getLibraries();

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
	QStringList pathElements = path.split('/');
    QString libraryName = DBHelper::getLibraryName(pathElements.at(3).toInt());
    QString fileName = pathElements.at(5);

	QImage img(libraries.getPath(libraryName)+"/.yacreaderlibrary/covers/"+fileName);
	if (!img.isNull()) {
		QByteArray ba;
		QBuffer buffer(&ba);
		buffer.open(QIODevice::WriteOnly);
        img.save(&buffer, "JPG");
		response.write(ba,true);
	}
	else
	{
		response.setStatus(404,"not found");
		response.write("404 not found",true);
	}
}

