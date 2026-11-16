#ifndef ITEMUPDATECONTROLLERV2_H
#define ITEMUPDATECONTROLLERV2_H

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"

#include <QUuid>

// Changes fields of one comic or one folder:
//   PATCH /v2/library/{id}/comic/{id}   {"read": bool, "file_type": int}
//   PATCH /v2/library/{id}/folder/{id}  {"finished": bool, "completed": bool, "file_type": int}
//   PATCH /v3/library/{id}/comic/{id}   {"read": bool, "file_type": int, "rating": int}
// The response is the updated item as JSON; v3 sends the full comic JSON.
class ItemUpdateControllerV2 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(ItemUpdateControllerV2)
public:
    enum class Api {
        V2,
        V3,
    };

    explicit ItemUpdateControllerV2(Api api = Api::V2);

    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

    /** The library changed by the request, null if nothing changed */
    QUuid changedLibraryId;

private:
    Api api;
};

#endif // ITEMUPDATECONTROLLERV2_H
