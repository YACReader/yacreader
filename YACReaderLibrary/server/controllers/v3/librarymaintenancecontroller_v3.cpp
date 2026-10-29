#include "librarymaintenancecontroller_v3.h"

#include "libraries_update_coordinator.h"
#include "static.h"
#include "v3_response.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaObject>
#include <QThread>

#include <functional>

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;

namespace {

void writeJson(HttpResponse &response, int status, const char *statusText, const QJsonObject &object)
{
    response.setStatus(status, statusText);
    V3Response::json(response, object);
}

bool runInCoordinatorThread(LibrariesUpdateCoordinator *coordinator, const std::function<void()> &call)
{
    if (coordinator->thread() == QThread::currentThread()) {
        call();
        return true;
    }
    return QMetaObject::invokeMethod(coordinator, call, Qt::BlockingQueuedConnection);
}

}

LibraryMaintenanceControllerV3::LibraryMaintenanceControllerV3(std::optional<int> recoveredLibraryId)
    : recoveredLibraryId(recoveredLibraryId)
{
}

void LibraryMaintenanceControllerV3::service(HttpRequest &request, HttpResponse &response)
{
    auto coordinator = Static::librariesUpdateCoordinator;
    if (coordinator == nullptr)
        return writeJson(response, 503, "Service Unavailable", { { "error", "updates_unavailable" } });

    if (!recoveredLibraryId) {
        QJsonObject status;
        if (!runInCoordinatorThread(coordinator, [&] { status = { { "running", coordinator->isRunning() }, { "problems", coordinator->lockProblems() } }; }))
            return writeJson(response, 503, "Service Unavailable", { { "error", "updates_unavailable" } });
        return V3Response::json(response, status);
    }

    const auto recoveryToken = QJsonDocument::fromJson(request.getBody()).object().value("recovery_token").toString();
    if (recoveryToken.isEmpty())
        return V3Response::badRequest(response);

    auto result = LibrariesUpdateCoordinator::RecoveryResult::Failed;
    QString error;
    if (!runInCoordinatorThread(coordinator, [&] { result = coordinator->recoverLibraryLock(*recoveredLibraryId, recoveryToken, &error); }))
        return writeJson(response, 503, "Service Unavailable", { { "error", "updates_unavailable" } });

    switch (result) {
    case LibrariesUpdateCoordinator::RecoveryResult::Recovered:
        return V3Response::json(response, QJsonObject { { "status", "recovered" } });
    case LibrariesUpdateCoordinator::RecoveryResult::AlreadyRunning:
        return writeJson(response, 409, "Conflict", { { "status", "already_running" }, { "running", true } });
    case LibrariesUpdateCoordinator::RecoveryResult::NotAllowed:
        return writeJson(response, 409, "Conflict", { { "status", "update_not_allowed" }, { "running", false } });
    case LibrariesUpdateCoordinator::RecoveryResult::LibraryNotFound:
        return V3Response::notFound(response);
    case LibrariesUpdateCoordinator::RecoveryResult::NotRecoverable:
        return writeJson(response, 409, "Conflict", { { "status", "not_recoverable" } });
    case LibrariesUpdateCoordinator::RecoveryResult::Failed:
        return writeJson(response, 500, "Internal Server Error", { { "status", "recovery_failed" }, { "error", error } });
    }
}
