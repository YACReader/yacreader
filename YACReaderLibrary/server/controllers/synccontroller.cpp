#include "synccontroller.h"

#include "QsLog.h"
#include <QUrl>

SyncController::SyncController()
{

}

void SyncController::service(HttpRequest &request, HttpResponse &response)
{
    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');
    qulonglong libraryId = pathElements.at(2).toULongLong();

    QString postData = QString::fromUtf8(request.getBody());

    QLOG_INFO() << "POST DATA: " << postData;

    //TODO Process postData and update the comics

    response.write("OK",true);
}

