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
    connect(&tT, &QTimer::timeout, &q, &QEventLoop::quit);
    connect(&manager, &QNetworkAccessManager::finished, &q, &QEventLoop::quit);

    auto request = QNetworkRequest(url);

    request.setHeader(QNetworkRequest::UserAgentHeader,
                      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 Chrome/122.0 Safari/537.36");

    QNetworkReply *reply = manager.get(request);

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
