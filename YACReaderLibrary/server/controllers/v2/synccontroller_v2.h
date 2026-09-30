#ifndef SYNCCONTROLLER_V2_H
#define SYNCCONTROLLER_V2_H

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"

#include <QList>
#include <QObject>
#include <QUuid>

class SyncControllerV2 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(SyncControllerV2)
public:
    /** Constructor */
    SyncControllerV2();

    /** Generates the response */
    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

    /** The libraries that received reading progress from the client */
    QList<QUuid> changedLibraries;

private:
    void addChangedLibrary(const QUuid &libraryId);
};

#endif // SYNCCONTROLLER_H
