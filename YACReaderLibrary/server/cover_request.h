#ifndef COVER_REQUEST_H
#define COVER_REQUEST_H

#include "httprequest.h"
#include "httpresponse.h"

#include <QString>

#include <optional>

// the file of a cover request: `fileName` under the covers folder of the library; nothing when it resolves outside it
std::optional<QString> coverRequestFile(const QString &libraryPath, const QString &fileName);

// false when there is no cover to send, and then nothing is written to the response
bool serveCover(const stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response, const QString &libraryPath, const QString &fileName);

#endif // COVER_REQUEST_H
