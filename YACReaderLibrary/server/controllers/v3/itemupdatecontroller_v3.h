#ifndef ITEMUPDATECONTROLLERV3_H
#define ITEMUPDATECONTROLLERV3_H

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"
#include "library_ids.h"

#include <QUuid>

#include <variant>

// the library itself, its id is the library id
struct LibraryItem {
};

using UpdatedItemId = std::variant<ComicId, FolderId, LibraryItem>;

// Changes fields of one comic, one folder or the library:
//   PATCH /v3/library/{id}/comic/{id}   {"read": bool, "file_type": int, "rating": int}
//   PATCH /v3/library/{id}/folder/{id}  {"finished": bool, "completed": bool, "file_type": int}
//   PATCH /v3/library/{id}              {"file_type": int}
// The response is the updated item as JSON, the full comic JSON for a comic.
class ItemUpdateControllerV3 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(ItemUpdateControllerV3)
public:
    ItemUpdateControllerV3(int libraryId, UpdatedItemId itemId);

    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

    /** The library changed by the request, null if nothing changed */
    QUuid changedLibraryId;

private:
    int libraryId;
    UpdatedItemId itemId;
};

#endif // ITEMUPDATECONTROLLERV3_H
