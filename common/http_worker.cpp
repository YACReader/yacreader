#include "http_worker.h"

#include <QUrl>
#include <QtGlobal>
#include <QStringList>

#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QTimer>
#include <QNetworkRequest>
#include <QNetworkReply>

#define PREVIOUS_VERSION "6.0.0"

HttpWorker::HttpWorker(const QString &urlString)
    : QThread(), url(urlString), _error(false), _timeout(false)
{
}

void HttpWorker::get()
{
    this->start();
}

QByteArray HttpWorker::getResult()
{
    return result;
}

bool HttpWorker::wasValid()
{
    return !_error;
}

bool HttpWorker::wasTimeout()
{
    return _timeout;
}

void HttpWorker::run()
{
    QNetworkAccessManager manager;
    QEventLoop q;
    QTimer tT;

    tT.setSingleShot(true);
    connect(&tT, SIGNAL(timeout()), &q, SLOT(quit()));
    connect(&manager, SIGNAL(finished(QNetworkReply *)), &q, SLOT(quit()));
    QNetworkReply *reply = manager.get(QNetworkRequest(url));

    tT.start(5000); // 5s timeout
    q.exec();

    if (tT.isActive()) {
        // download complete
        _error = !(reply->error() == QNetworkReply::NoError);
        result = reply->readAll();
        emit dataReady(result);
        tT.stop();
    } else {
        _timeout = true;
        emit timeout();
    }
}
