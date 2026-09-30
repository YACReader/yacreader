#ifndef REQUESTMAPPER_H
#define REQUESTMAPPER_H

#include "httprequesthandler.h"

#include <QMutex>
#include <QUuid>

class RequestMapper : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(RequestMapper)
public:
    RequestMapper(QObject *parent = nullptr);

    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;
    void loadSessionV2(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response);

signals:
    void comicUpdated(qulonglong libraryId, qulonglong comicId);
    // a remote client changed the content of a library (reading progress, read status, type...)
    void libraryContentChanged(const QUuid &libraryId);

private:
    void serviceV2(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response);
    void serviceWebUI(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response);

    static QMutex mutex;
};

#endif // REQUESTMAPPER_H
