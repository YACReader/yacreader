#ifndef LISTCONTENTCONTROLLER_V3_H
#define LISTCONTENTCONTROLLER_V3_H

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"
#include "library_ids.h"

#include <variant>

using ContentListId = std::variant<ReadingListId, LabelId, SmartListId>;

// the entries of a reading list or a label (comics, deleted comics, and placeholders) in their order, or the comics of
// a smart list in the search order
class ListContentControllerV3 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(ListContentControllerV3)
public:
    ListContentControllerV3(int libraryId, ContentListId listId);

    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

private:
    int libraryId;
    ContentListId listId;
};

#endif // LISTCONTENTCONTROLLER_V3_H
