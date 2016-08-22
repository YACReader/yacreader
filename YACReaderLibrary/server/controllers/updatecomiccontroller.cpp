#include "updatecomiccontroller.h"

#include "db_helper.h"
#include "yacreader_libraries.h"

#include "template.h"
#include "../static.h"

#include "comic_db.h"
#include "comic.h"

#include "QsLog.h"

UpdateComicController::UpdateComicController(){}

void UpdateComicController::service(HttpRequest &request, HttpResponse &response)
{
    HttpSession session=Static::sessionStore->getSession(request,response,false);

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');
    qulonglong libraryId = pathElements.at(2).toULongLong();
    QString libraryName = DBHelper::getLibraryName(libraryId);
    qulonglong comicId = pathElements.at(4).toULongLong();

    QString postData = QString::fromUtf8(request.getBody());

    QLOG_TRACE() << "POST DATA: " << postData;

    if(postData.length()>0) {
        QList<QString> data = postData.split("\n");
        int currentPage = data.at(0).split(":").at(1).toInt();
        ComicInfo info;
        info.currentPage = currentPage;
        info.id = comicId;
        DBHelper::updateProgress(libraryId,info);
    }
    else
    {
        response.setStatus(412,"No comic info received");
        response.write("",true);
        return;
    }

    response.write("OK",true);
}
