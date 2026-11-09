#ifndef LIBRARYMAINTENANCECONTROLLER_V3_H
#define LIBRARYMAINTENANCECONTROLLER_V3_H

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"

#include <optional>

// without a library: the running state and the lock problems of library updates and XML rescans
// with a library: the explicit recovery of that library's stale maintenance lock
class LibraryMaintenanceControllerV3 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(LibraryMaintenanceControllerV3)
public:
    explicit LibraryMaintenanceControllerV3(std::optional<int> recoveredLibraryId = std::nullopt);

    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

private:
    std::optional<int> recoveredLibraryId;
};

#endif // LIBRARYMAINTENANCECONTROLLER_V3_H
