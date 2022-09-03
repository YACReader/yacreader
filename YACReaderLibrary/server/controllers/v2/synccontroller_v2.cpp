#include "synccontroller_v2.h"

#include "QsLog.h"
#include <QUrl>

#include "comic_db.h"
#include "db_helper.h"
#include "yacreader_server_data_helper.h"

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;

SyncControllerV2::SyncControllerV2()
{
}

void SyncControllerV2::service(HttpRequest &request, HttpResponse &response)
{
    response.setHeader("Content-Type", "text/plain; charset=utf-8");

    QString postData = QString::fromUtf8(request.getBody());

    QLOG_TRACE() << "POST DATA: " << postData;

    if (postData.length() > 0) {
        QList<QString> data = postData.split("\n");

        qulonglong libraryId;
        qulonglong comicId;
        int currentPage;
        int currentRating;
        unsigned long long lastTimeOpened;
        QString hash;
        QMap<qulonglong, QList<ComicInfo>> comics;
        QList<ComicInfo> comicsWithNoLibrary;
        foreach (QString comicInfo, data) {
            QList<QString> comicInfoProgress = comicInfo.split("\t");

            if (comicInfoProgress.length() >= 6) {
                if (comicInfoProgress.at(0) != "unknown") {
                    libraryId = comicInfoProgress.at(0).toULongLong();
                    comicId = comicInfoProgress.at(1).toULongLong();
                    hash = comicInfoProgress.at(2);
                    currentPage = comicInfoProgress.at(3).toInt();

                    ComicInfo info;
                    info.currentPage = currentPage;
                    info.hash = hash; // TODO remove the hash check and add UUIDs for libraries
                    info.id = comicId;

                    currentRating = comicInfoProgress.at(4).toInt();
                    info.rating = currentRating;

                    lastTimeOpened = comicInfoProgress.at(5).toULong();
                    info.lastTimeOpened = lastTimeOpened;

                    if (comicInfoProgress.length() >= 7) {
                        info.read = comicInfoProgress.at(6).toInt();
                    }

                    if (!comics.contains(libraryId)) {
                        comics[libraryId] = QList<ComicInfo>();
                    }
                    comics[libraryId].push_back(info);
                } else {
                    hash = comicInfoProgress.at(2);
                    currentPage = comicInfoProgress.at(3).toInt();

                    ComicInfo info;
                    info.currentPage = currentPage;
                    info.hash = hash;

                    currentRating = comicInfoProgress.at(4).toInt();
                    info.rating = currentRating;

                    lastTimeOpened = comicInfoProgress.at(5).toULong();
                    info.lastTimeOpened = lastTimeOpened;

                    comicsWithNoLibrary.push_back(info);
                }
            }
        }

        auto moreRecentComicsFound = DBHelper::updateFromRemoteClient(comics);

        QJsonArray items;

        foreach (qulonglong libraryId, moreRecentComicsFound.keys()) {
            foreach (ComicDB comic, moreRecentComicsFound[libraryId]) {
                items.append(YACReaderServerDataHelper::comicToJSON(libraryId, comic));
            }
        }

        QJsonDocument output(items);

        response.write(output.toJson(QJsonDocument::Compact), true);

        // TODO does it make sense to send these back? The source is not YACReaderLibrary...
        DBHelper::updateFromRemoteClientWithHash(comicsWithNoLibrary);

    } else {
        response.setStatus(412, "No comic info received");
        response.write("[]", true);
        return;
    }
}
