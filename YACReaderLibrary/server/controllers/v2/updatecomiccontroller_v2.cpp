#include "updatecomiccontroller_v2.h"

#include "QsLog.h"
#include "comic_db.h"
#include "db_helper.h"

#include <QUrl>

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;

UpdateComicControllerV2::UpdateComicControllerV2() { }

void UpdateComicControllerV2::service(HttpRequest &request, HttpResponse &response)
{
    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');
    qulonglong libraryId = pathElements.at(3).toULongLong();
    qulonglong comicId = pathElements.at(5).toULongLong();

    QString postData = QString::fromUtf8(request.getBody());

    QLOG_TRACE() << "POST DATA: " << postData;

    if (postData.length() > 0) {
        QList<QString> data = postData.split("\n");
        QString currentComicData = data.at(0);
        int currentPage = currentComicData.split(":").at(1).toInt();
        ComicInfo info;
        info.currentPage = currentPage;

        if (data.length() > 1) {
            if (data.at(1).isEmpty() == false) {
                QString nextComicId = data.at(1);
                DBHelper::setComicAsReading(libraryId, ComicId(nextComicId.toULongLong()));
            }

            if (data.length() > 2) {
                QStringList imageFiltersData = data.at(2).split("\t");
                auto epoch = imageFiltersData.at(0).toULongLong();
                QString imageFiltersJson = imageFiltersData.at(1);

                ComicInfo imageFiltersInfo;
                imageFiltersInfo.lastTimeImageFiltersSet = epoch;
                imageFiltersInfo.imageFiltersJson = imageFiltersJson.isEmpty() ? QVariant() : imageFiltersJson;

                DBHelper::updateImageFilters(libraryId, ComicId(comicId), imageFiltersInfo);
            }
        }

        DBHelper::updateProgress(libraryId, ComicId(comicId), info);

        error = false;
        updatedLibraryId = libraryId;
        updatedComicId = comicId;
    } else {
        error = true;
        response.setStatus(412, "No comic info received");
        response.write("", true);
        return;
    }

    response.write("OK", true);
}
