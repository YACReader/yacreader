#include "v3_response.h"

#include <QJsonDocument>

using stefanfrings::HttpResponse;

namespace V3Response {

void json(HttpResponse &response, const QJsonArray &items)
{
    response.setHeader("Content-Type", "application/json");
    response.write(QJsonDocument(items).toJson(QJsonDocument::Compact), true);
}

void json(HttpResponse &response, const QJsonObject &item)
{
    response.setHeader("Content-Type", "application/json");
    response.write(QJsonDocument(item).toJson(QJsonDocument::Compact), true);
}

void badRequest(HttpResponse &response)
{
    response.setStatus(400, "Bad Request");
    response.write("400 bad request", true);
}

void notFound(HttpResponse &response)
{
    response.setStatus(404, "not found");
    response.write("404 not found", true);
}

void methodNotAllowed(HttpResponse &response)
{
    response.setStatus(405, "Method Not Allowed");
    response.write("405 method not allowed", true);
}

void serverError(HttpResponse &response)
{
    response.setStatus(500, "Internal Server Error");
    response.write("500 internal server error", true);
}

}
