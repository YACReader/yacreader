#include "updatelibrariescontroller_v2.h"

#include "libraries_update_coordinator.h"
#include "static.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaObject>
#include <QRegExp>
#include <QThread>

#include <optional>

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;

namespace {

void writeJson(HttpResponse &response, int status, const char *statusText, const QJsonObject &object)
{
    response.setStatus(status, statusText);
    response.setHeader("Content-Type", "application/json");
    response.write(QJsonDocument(object).toJson(QJsonDocument::Compact), true);
}

void methodNotAllowed(HttpResponse &response, const QByteArray &allowedMethods)
{
    response.setHeader("Allow", allowedMethods);
    writeJson(response, 405, "Method Not Allowed", { { "error", "method_not_allowed" } });
}

std::optional<LibrariesUpdateCoordinator::UpdateRequestResult> requestUpdate(LibrariesUpdateCoordinator *coordinator, const std::optional<int> &libraryId)
{
    LibrariesUpdateCoordinator::UpdateRequestResult result = LibrariesUpdateCoordinator::UpdateRequestResult::NotAllowed;
    const auto request = [coordinator, libraryId, &result] {
        result = libraryId.has_value()
                ? coordinator->requestSingleLibraryUpdate(libraryId.value())
                : coordinator->requestLibrariesUpdate();
    };

    if (coordinator->thread() == QThread::currentThread()) {
        request();
    } else if (!QMetaObject::invokeMethod(coordinator, request, Qt::BlockingQueuedConnection)) {
        return std::nullopt;
    }

    return result;
}

} // namespace

UpdateLibrariesControllerV2::UpdateLibrariesControllerV2() { }

void UpdateLibrariesControllerV2::service(HttpRequest &request, HttpResponse &response)
{
    auto coordinator = Static::librariesUpdateCoordinator;
    if (coordinator == nullptr) {
        writeJson(response, 503, "Service Unavailable", { { "error", "updates_unavailable" } });
        return;
    }

    const QByteArray path = request.getPath();
    const QByteArray method = request.getMethod();

    // GET /v2/libraries/update/status
    if (path.contains("/update/status")) {
        if (method != "GET") {
            methodNotAllowed(response, "GET");
            return;
        }

        writeJson(response, 200, "OK", { { "running", coordinator->isRunning() } });
        return;
    }

    // POST /v2/libraries/update/cancel
    if (path.contains("/update/cancel")) {
        if (method != "POST") {
            methodNotAllowed(response, "POST");
            return;
        }

        QMetaObject::invokeMethod(coordinator, "cancel", Qt::QueuedConnection);
        writeJson(response, 202, "Accepted", { { "status", "canceling" } });
        return;
    }

    // Trigger endpoints: POST .../update
    if (method != "POST") {
        methodNotAllowed(response, "POST");
        return;
    }

    QRegExp singleLibrary("/v2/library/([0-9]+)/update/?");
    std::optional<int> libraryId;
    if (singleLibrary.exactMatch(QString::fromUtf8(path))) {
        libraryId = singleLibrary.cap(1).toInt();
    }

    const auto result = requestUpdate(coordinator, libraryId);
    if (!result.has_value()) {
        writeJson(response, 503, "Service Unavailable", { { "error", "updates_unavailable" } });
        return;
    }

    switch (result.value()) {
    case LibrariesUpdateCoordinator::UpdateRequestResult::Started:
        writeJson(response, 202, "Accepted", { { "status", "started" }, { "running", true } });
        return;
    case LibrariesUpdateCoordinator::UpdateRequestResult::AlreadyRunning:
        writeJson(response, 409, "Conflict", { { "status", "already_running" }, { "running", true } });
        return;
    case LibrariesUpdateCoordinator::UpdateRequestResult::NotAllowed:
        writeJson(response, 409, "Conflict", { { "status", "update_not_allowed" }, { "running", false } });
        return;
    case LibrariesUpdateCoordinator::UpdateRequestResult::LibraryNotFound:
        writeJson(response, 404, "Not Found", { { "error", "library_not_found" } });
        return;
    }
}
