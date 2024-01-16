#ifndef FOLDERMETADATACONTROLLERV2_H
#define FOLDERMETADATACONTROLLERV2_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class FolderMetadataControllerV2 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(FolderMetadataControllerV2)
public:
    /** Constructor */
    FolderMetadataControllerV2();

    /** Generates the response */
    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;
};

#endif // FOLDERMETADATACONTROLLERV2_H
