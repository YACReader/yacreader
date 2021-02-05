#include "updatecomiccontroller_v2.h"

#include "db_helper.h"
#include "yacreader_libraries.h"

#include "template.h"
#include "../static.h"

#include "comic_db.h"
#include "comic.h"

#include "QsLog.h"

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
        info.id = comicId;
        DBHelper::updateProgress(libraryId, info);

        if (data.length() > 1) {
            if (data.at(1).isEmpty() == false) {
                QString nextComicId = data.at(1);
                ComicInfo info;
                info.id = nextComicId.toULongLong();
                DBHelper::setComicAsReading(libraryId, info);
            }
        }
    } else {
        response.setStatus(412, "No comic info received");
        response.write("", true);
        return;
    }

    response.write("OK", true);
}
