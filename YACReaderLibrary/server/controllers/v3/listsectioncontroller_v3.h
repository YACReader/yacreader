#ifndef LISTSECTIONCONTROLLER_V3_H
#define LISTSECTIONCONTROLLER_V3_H

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"
#include "library_ids.h"

#include <optional>

// one folder of the reading lists or the smart lists section (nothing: the top level): its folders, then its lists,
// in the order of the sidebar
class ListSectionControllerV3 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(ListSectionControllerV3)
public:
    ListSectionControllerV3(int libraryId, ListFolderKind kind, std::optional<qulonglong> folderId);

    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

private:
    int libraryId;
    ListFolderKind kind;
    std::optional<qulonglong> folderId;
};

#endif // LISTSECTIONCONTROLLER_V3_H
