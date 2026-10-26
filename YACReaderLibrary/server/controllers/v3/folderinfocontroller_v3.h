#ifndef FOLDERINFOCONTROLLER_V3_H
#define FOLDERINFOCONTROLLER_V3_H

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"
#include "library_ids.h"

// the comics and placeholders of a folder and of all its subfolders, for imports: each folder in the order of the
// desktop views, then its subfolders by name
class FolderInfoControllerV3 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(FolderInfoControllerV3)
public:
    FolderInfoControllerV3(int libraryId, FolderId folderId);

    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

private:
    int libraryId;
    FolderId folderId;
};

#endif // FOLDERINFOCONTROLLER_V3_H
