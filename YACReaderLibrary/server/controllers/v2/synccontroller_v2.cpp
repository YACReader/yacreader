#include "synccontroller_v2.h"

#include "QsLog.h"
#include <QUrl>

#include "comic_db.h"
#include "db_helper.h"

SyncControllerV2::SyncControllerV2()
{

}

void SyncControllerV2::service(HttpRequest &request, HttpResponse &response)
{
    QString postData = QString::fromUtf8(request.getBody());

    QLOG_TRACE() << "POST DATA: " << postData;

    if(postData.length()>0) {
        QList<QString> data = postData.split("\n");

        qulonglong libraryId;
        qulonglong comicId;
        int currentPage;
        int currentRating;
        unsigned long long lastTimeOpened;
        QString hash;
        foreach(QString comicInfo, data)
        {
            QList<QString> comicInfoProgress = comicInfo.split("\t");

            if(comicInfoProgress.length() == 6)
            {
                if (comicInfoProgress.at(0) == "unknown")
                {
                    libraryId = comicInfoProgress.at(0).toULongLong();
                    comicId = comicInfoProgress.at(1).toULongLong();
                    hash = comicInfoProgress.at(2);
                    currentPage = comicInfoProgress.at(3).toInt();

                    ComicInfo info;
                    info.currentPage = currentPage;
                    info.hash = hash; //TODO remove the hash check and add UUIDs for libraries
                    info.id = comicId;

                    currentRating = comicInfoProgress.at(4).toInt();
                    info.rating = currentRating;

                    lastTimeOpened = comicInfoProgress.at(5).toULong();
                    info.lastTimeOpened = lastTimeOpened;

                    DBHelper::updateFromRemoteClient(libraryId,info);
                }
                else
                {
                    hash = comicInfoProgress.at(2);
                    currentPage = comicInfoProgress.at(3).toInt();

                    ComicInfo info;
                    info.currentPage = currentPage;
                    info.hash = hash; //TODO remove the hash check and add UUIDs for libraries

                    currentRating = comicInfoProgress.at(4).toInt();
                    info.rating = currentRating;

                    lastTimeOpened = comicInfoProgress.at(5).toULong();
                    info.lastTimeOpened = lastTimeOpened;

                    DBHelper::updateFromRemoteClientWithHash(info);
                }
            }
        }
    }
    else
    {
        response.setStatus(412,"No comic info received");
        response.write("",true);
        return;
    }

    response.write("OK",true);
}

