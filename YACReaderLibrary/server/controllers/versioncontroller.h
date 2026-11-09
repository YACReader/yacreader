#ifndef VERSIONCONTROLLER_H
#define VERSIONCONTROLLER_H

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"

#include <QThread>

class VersionController : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(VersionController);

public:
    explicit VersionController(const QByteArray &version);

    /** Generates the response */
    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

private:
    QByteArray version;
};

#endif // VERSIONCONTROLLER_H
