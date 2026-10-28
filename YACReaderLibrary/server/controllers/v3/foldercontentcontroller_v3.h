#ifndef FOLDERCONTENTCONTROLLER_V3_H
#define FOLDERCONTENTCONTROLLER_V3_H

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"
#include "library_ids.h"

// the subfolders, comics, and placeholders of a folder, in the order of the desktop views
class FolderContentControllerV3 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(FolderContentControllerV3)
public:
    FolderContentControllerV3(int libraryId, FolderId folderId);

    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

private:
    int libraryId;
    FolderId folderId;
};

#endif // FOLDERCONTENTCONTROLLER_V3_H
