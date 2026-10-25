#include "tagscontroller_v3.h"

#include "data_base_management.h"
#include "db_helper.h"
#include "reading_list.h"
#include "v3_response.h"
#include "yacreader_global.h"
#include "yacreader_libraries.h"
#include "yacreader_server_data_helper.h"

#include <QJsonArray>

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;

TagsControllerV3::TagsControllerV3(int libraryId)
    : libraryId(libraryId)
{
}

void TagsControllerV3::service(HttpRequest & /* request */, HttpResponse &response)
{
    const auto libraries = DBHelper::getLibraries();
    const auto libraryUuid = libraries.getLibraryIdFromLegacyId(libraryId);

    std::optional<QList<Label>> labels;
    QString connectionName;
    {
        auto db = DataBaseManagement::loadDatabase(YACReader::LibraryPaths::libraryDataPath(libraries.getPath(libraryId)));
        labels = DBHelper::loadLabels(db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (!labels)
        return V3Response::serverError(response);

    QJsonArray items;
    for (const auto &label : std::as_const(*labels))
        items.append(YACReaderServerDataHelper::labelToJSON(libraryId, libraryUuid, label));
    V3Response::json(response, items);
}
