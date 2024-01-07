#include "foldermetadatacontroller_v2.h"

#include "db_helper.h"
#include "folder.h"

#include "yacreader_libraries.h"
#include "yacreader_server_data_helper.h"

FolderMetadataControllerV2::FolderMetadataControllerV2() { }

void FolderMetadataControllerV2::service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response)
{
    response.setHeader("Content-Type", "application/json");

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');
    int libraryId = pathElements.at(3).toInt();
    qulonglong folderId = pathElements.at(5).toULongLong();

    auto folder = DBHelper::getFolder(libraryId, folderId);
    if (!folder.knownId) {
        response.setStatus(404, "not found");
        response.write("404 not found", true);
        return;
    }

    auto libraryUuid = DBHelper::getLibraries().getLibraryIdFromLegacyId(libraryId);

    auto json = YACReaderServerDataHelper::folderToJSON(libraryId, libraryUuid, folder);

    QJsonDocument output(json);

    response.write(output.toJson(QJsonDocument::Compact));
}
