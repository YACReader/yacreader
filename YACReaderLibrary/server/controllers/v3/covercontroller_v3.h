#ifndef COVERCONTROLLER_V3_H
#define COVERCONTROLLER_V3_H

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"

// a file under the covers folder: comic covers by hash, `folders/` for folders, `placeholders/` for placeholders
class CoverControllerV3 : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(CoverControllerV3)
public:
    CoverControllerV3(int libraryId, const QString &fileName);

    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

private:
    int libraryId;
    QString fileName;
};

#endif // COVERCONTROLLER_V3_H
