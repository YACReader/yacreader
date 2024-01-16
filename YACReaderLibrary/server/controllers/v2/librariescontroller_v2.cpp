#include "librariescontroller_v2.h"
#include "db_helper.h" //get libraries
#include "yacreader_libraries.h"

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;

LibrariesControllerV2::LibrariesControllerV2() { }

void LibrariesControllerV2::service(HttpRequest & /* request */, HttpResponse &response)
{
    response.setHeader("Content-Type", "application/json");

    auto libraries = DBHelper::getLibraries().sortedLibraries();

    QJsonArray librariesJson;

    foreach (YACReaderLibrary library, libraries) {
        QJsonObject libraryJson;

        libraryJson["name"] = library.getName();
        libraryJson["id"] = library.getLegacyId();
        libraryJson["uuid"] = library.getId().toString();

        librariesJson.append(libraryJson);
    }

    QJsonDocument output(librariesJson);

    response.setStatus(200, "OK");
    response.write(output.toJson(QJsonDocument::Compact), true);
}
