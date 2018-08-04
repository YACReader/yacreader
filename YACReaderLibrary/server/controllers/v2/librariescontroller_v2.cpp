#include "librariescontroller_v2.h"
#include "db_helper.h"  //get libraries
#include "yacreader_libraries.h"

#include "template.h"
#include "../static.h"

#include "QsLog.h"

LibrariesControllerV2::LibrariesControllerV2() {}

void LibrariesControllerV2::service(HttpRequest& request, HttpResponse& response)
{
    response.setHeader("Content-Type", "application/json");

	YACReaderLibraries libraries = DBHelper::getLibraries();
	QList<QString> names = DBHelper::getLibrariesNames();

    QJsonArray librariesJson;

	int currentId = 0;
	foreach (QString name,names) {
        currentId = libraries.getId(name);
        QJsonObject library;

        library["name"] = name;
        library["id"] = currentId;

        librariesJson.append(library);
	}

    QJsonDocument output(librariesJson);

    response.setStatus(200,"OK");
    response.write(output.toJson(QJsonDocument::Compact),true);
}
