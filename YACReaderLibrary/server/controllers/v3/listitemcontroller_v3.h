#ifndef LISTITEMCONTROLLER_V3_H
#define LISTITEMCONTROLLER_V3_H

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"
#include "library_ids.h"

#include <variant>

using SectionItemId = std::variant<ReadingListId, SmartListId, ListFolderId>;

// one reading list, smart list, or list folder, as the lists sections send it
class ListItemControllerV3 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(ListItemControllerV3)
public:
    ListItemControllerV3(int libraryId, SectionItemId itemId);

    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

private:
    int libraryId;
    SectionItemId itemId;
};

#endif // LISTITEMCONTROLLER_V3_H
