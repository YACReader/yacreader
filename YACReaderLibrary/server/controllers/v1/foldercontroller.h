#ifndef FOLDERCONTROLLER_H
#define FOLDERCONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class FolderController : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(FolderController);

public:
    /** Constructor */
    FolderController();

    /** Generates the response */
    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;
};

#endif // FOLDERCONTROLLER_H
