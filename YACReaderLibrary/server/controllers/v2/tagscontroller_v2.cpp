#include "tagscontroller_v2.h"

#include "db_helper.h"
#include "yacreader_libraries.h"

#include "reading_list.h"

#include "yacreader_server_data_helper.h"

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;

TagsControllerV2::TagsControllerV2() { }

void TagsControllerV2::service(HttpRequest &request, HttpResponse &response)
{
    response.setHeader("Content-Type", "text/plain; charset=utf-8");

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');
    int libraryId = pathElements.at(3).toInt();

    QList<Label> labels = DBHelper::getLabels(libraryId);

    auto libraryUuid = DBHelper::getLibraries().getLibraryIdFromLegacyId(libraryId);

    QJsonArray items;

    for (QList<Label>::const_iterator itr = labels.constBegin(); itr != labels.constEnd(); itr++) {
        items.append(YACReaderServerDataHelper::labelToJSON(libraryId, libraryUuid, *itr));
    }

    QJsonDocument output(items);

    response.write(output.toJson(QJsonDocument::Compact));
    ;
}
