#include "synccontroller.h"

#include "QsLog.h"
#include <QUrl>

#include "comic_db.h"
#include "db_helper.h"

SyncController::SyncController()
{

}

void SyncController::service(HttpRequest &request, HttpResponse &response)
{
    QString postData = QString::fromUtf8(request.getBody());

    QLOG_INFO() << "POST DATA: " << postData;

    if(postData.length()>0) {
        QList<QString> data = postData.split("\n");

        qulonglong libraryId;
        qulonglong comicId;
        int currentPage;
        QString hash;
        foreach(QString comicInfo, data)
        {
            QList<QString> comicInfoProgress = comicInfo.split("\t");

            if(comicInfoProgress.length() == 4)
            {
                libraryId = comicInfoProgress.at(0).toULongLong();
                comicId = comicInfoProgress.at(1).toULongLong();
                hash = comicInfoProgress.at(2);
                currentPage = comicInfoProgress.at(3).toInt();

                ComicInfo info;
                info.currentPage = currentPage;
                info.hash = hash; //TODO remove the hash check and add UUIDs for libraries
                info.id = comicId;
                DBHelper::updateFromRemoteClient(libraryId,info);
            }
        }
    }
    else
    {
        response.setStatus(412,"No comic info received");
        response.writeText("",true);
        return;
    }

    response.write("OK",true);
}

