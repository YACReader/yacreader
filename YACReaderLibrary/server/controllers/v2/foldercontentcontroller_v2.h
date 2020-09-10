#ifndef FOLDERCONTENTCONTROLLER_V2_H
#define FOLDERCONTENTCONTROLLER_V2_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class FolderContentControllerV2 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(FolderContentControllerV2)
public:
    /** Constructor */
    FolderContentControllerV2();

    /** Generates the response */
    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

private:
    void serviceContent(const int &library, const qulonglong &folderId, stefanfrings::HttpResponse &response);
};

#endif // FOLDERCONTENTCONTROLLER_H
