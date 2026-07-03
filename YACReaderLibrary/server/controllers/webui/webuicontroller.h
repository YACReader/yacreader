#ifndef WEBUI_CONTROLLER_H
#define WEBUI_CONTROLLER_H

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"

#include <QString>

class WebUIController : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(WebUIController);

public:
    WebUIController();

    void service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response) override;

private:
    void renderStatusPage(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response);
    void renderLibraryBrowser(stefanfrings::HttpRequest &request,
                              stefanfrings::HttpResponse &response,
                              int libraryId,
                              const QString &libraryName,
                              const QString &initialView,
                              qulonglong initialItemId);
    void renderSettingsPage(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response, const QString &errorMessage = QString());
    bool saveSettings(stefanfrings::HttpRequest &request, QString &errorMessage, int &errorStatus);
};

#endif // WEBUI_CONTROLLER_H
