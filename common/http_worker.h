#ifndef __HTTP_WORKER_H
#define __HTTP_WORKER_H

#include <QByteArray>
#include <QThread>
#include <QUrl>

class HttpWorker : public QThread
{
    Q_OBJECT
public:
    HttpWorker(const QString &urlString, const QString &userAgent);
public slots:
    void get();
    QByteArray getResult();
    bool wasValid();
    bool wasTimeout();

private:
    void run();
    QUrl url;
    QString userAgent;
    int httpGetId;
    QByteArray result;
    bool _error;
    bool _timeout;
signals:
    void dataReady(const QByteArray &);
    void timeout();
};

#endif
