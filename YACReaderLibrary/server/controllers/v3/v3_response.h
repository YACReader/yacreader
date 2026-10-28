#ifndef V3_RESPONSE_H
#define V3_RESPONSE_H

#include "httpresponse.h"

#include <QJsonArray>
#include <QJsonObject>

namespace V3Response {

void json(stefanfrings::HttpResponse &response, const QJsonArray &items);
void json(stefanfrings::HttpResponse &response, const QJsonObject &item);
void badRequest(stefanfrings::HttpResponse &response);
void notFound(stefanfrings::HttpResponse &response);
void methodNotAllowed(stefanfrings::HttpResponse &response);
void serverError(stefanfrings::HttpResponse &response);

}

#endif // V3_RESPONSE_H
