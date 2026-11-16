#ifndef MISSINGCOMICCONTROLLER_V3_H
#define MISSINGCOMICCONTROLLER_V3_H

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"
#include "library_ids.h"

#include <variant>

using MissingComicId = std::variant<ComicInfoId, PlaceholderId>;

// one deleted comic or placeholder, as the content sends it, with all its descriptive fields
class MissingComicControllerV3 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(MissingComicControllerV3)
public:
    MissingComicControllerV3(int libraryId, MissingComicId itemId);

    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

private:
    int libraryId;
    MissingComicId itemId;
};

#endif // MISSINGCOMICCONTROLLER_V3_H
