#include "itemupdatecontroller_v2.h"

#include "comic_db.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "folder.h"
#include "yacreader_libraries.h"
#include "yacreader_server_data_helper.h"

#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlDatabase>
#include <QUrl>

#include <functional>
#include <optional>

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;

namespace {

struct UpdateResult {
    int status = 200;
    QByteArray statusText = "OK";
    QJsonDocument body;
};

UpdateResult errorResult(int status, const QByteArray &statusText, const QString &error, const QString &field = { })
{
    QJsonObject body { { "error", error } };
    if (!field.isEmpty()) {
        body["field"] = field;
    }
    return { status, statusText, QJsonDocument(body) };
}

UpdateResult databaseError()
{
    return errorResult(503, "Service Unavailable", QStringLiteral("database_unavailable"));
}

void writeResult(HttpResponse &response, const UpdateResult &result)
{
    response.setStatus(result.status, result.statusText);
    response.setHeader("Content-Type", "application/json");
    response.write(result.body.toJson(QJsonDocument::Compact), true);
}

enum class FieldType {
    Bool,
    FileType,
};

// The fields that clients can change. The names are the same as in the item JSON.
const QHash<QString, FieldType> &comicFields()
{
    static const QHash<QString, FieldType> fields {
        { QStringLiteral("read"), FieldType::Bool },
        { QStringLiteral("file_type"), FieldType::FileType },
    };
    return fields;
}

const QHash<QString, FieldType> &folderFields()
{
    static const QHash<QString, FieldType> fields {
        { QStringLiteral("finished"), FieldType::Bool },
        { QStringLiteral("completed"), FieldType::Bool },
        { QStringLiteral("file_type"), FieldType::FileType },
    };
    return fields;
}

bool isValidFileType(const QJsonValue &value)
{
    if (!value.isDouble()) {
        return false;
    }

    const auto number = value.toDouble();
    const auto type = value.toInt(-1);
    return number == type && type >= static_cast<int>(YACReader::FileType::Comic) && type <= static_cast<int>(YACReader::FileType::Yonkoma);
}

YACReader::FileType fileType(const QJsonValue &value)
{
    return static_cast<YACReader::FileType>(value.toInt());
}

// Checks the whole body before anything is written, so a bad request changes nothing.
std::optional<UpdateResult> validate(const QJsonObject &json, const QHash<QString, FieldType> &fields)
{
    if (json.isEmpty()) {
        return errorResult(400, "Bad Request", QStringLiteral("no_fields"));
    }

    for (auto it = json.constBegin(); it != json.constEnd(); ++it) {
        const auto field = fields.constFind(it.key());
        if (field == fields.constEnd()) {
            return errorResult(400, "Bad Request", QStringLiteral("unknown_field"), it.key());
        }

        const bool valid = field.value() == FieldType::Bool ? it.value().isBool() : isValidFileType(it.value());
        if (!valid) {
            return errorResult(400, "Bad Request", QStringLiteral("invalid_value"), it.key());
        }
    }

    return std::nullopt;
}

// Runs `apply` inside a transaction, it rolls back if `apply` fails or the commit fails.
bool runInTransaction(QSqlDatabase &db, const std::function<bool()> &apply)
{
    if (!db.transaction()) {
        return false;
    }

    if (!apply() || !db.commit()) {
        db.rollback();
        return false;
    }

    return true;
}

// `legacyLibraryId` is still needed in the JSON items because the clients use it.
UpdateResult updateComic(QSqlDatabase &db, int legacyLibraryId, const QUuid &libraryId, qulonglong comicId, const QJsonObject &json)
{
    if (auto error = validate(json, comicFields())) {
        return error.value();
    }

    bool found = false;
    DBHelper::loadComic(comicId, db, found);
    if (!found) {
        return errorResult(404, "Not Found", QStringLiteral("comic_not_found"));
    }

    const bool success = runInTransaction(db, [&] {
        if (json.contains("read") && !DBHelper::setComicRead(comicId, json.value("read").toBool(), db)) {
            return false;
        }
        if (json.contains("file_type") && !DBHelper::setComicType(comicId, fileType(json.value("file_type")), db)) {
            return false;
        }
        return true;
    });

    if (!success) {
        return databaseError();
    }

    const auto comic = DBHelper::loadComic(comicId, db, found);
    return { 200, "OK", QJsonDocument(YACReaderServerDataHelper::comicToJSON(legacyLibraryId, libraryId, comic)) };
}

UpdateResult updateFolder(QSqlDatabase &db, int legacyLibraryId, const QUuid &libraryId, qulonglong folderId, const QJsonObject &json)
{
    if (auto error = validate(json, folderFields())) {
        return error.value();
    }

    if (!DBHelper::loadFolder(folderId, db).knownId) {
        return errorResult(404, "Not Found", QStringLiteral("folder_not_found"));
    }

    const bool success = runInTransaction(db, [&] {
        if (json.contains("finished") && !DBHelper::setFolderFinished(folderId, json.value("finished").toBool(), db)) {
            return false;
        }
        if (json.contains("completed") && !DBHelper::setFolderCompleted(folderId, json.value("completed").toBool(), db)) {
            return false;
        }
        // the type applies to the folder and all its content, as in YACReaderLibrary
        if (json.contains("file_type") && !DBHelper::updateFolderTreeType(folderId, db, fileType(json.value("file_type")))) {
            return false;
        }
        return true;
    });

    if (!success) {
        return databaseError();
    }

    const auto folder = DBHelper::loadFolder(folderId, db);
    return { 200, "OK", QJsonDocument(YACReaderServerDataHelper::folderToJSON(legacyLibraryId, libraryId, folder)) };
}

} // namespace

ItemUpdateControllerV2::ItemUpdateControllerV2() { }

void ItemUpdateControllerV2::service(HttpRequest &request, HttpResponse &response)
{
    if (request.getMethod() != "PATCH") {
        response.setHeader("Allow", "PATCH");
        writeResult(response, errorResult(405, "Method Not Allowed", QStringLiteral("method_not_allowed")));
        return;
    }

    // A JSON content type forces a CORS preflight in browsers, so other web pages can not send these requests.
    if (!request.getHeader("content-type").toLower().startsWith("application/json")) {
        writeResult(response, errorResult(415, "Unsupported Media Type", QStringLiteral("json_required")));
        return;
    }

    QJsonParseError parseError;
    const auto json = QJsonDocument::fromJson(request.getBody(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !json.isObject()) {
        writeResult(response, errorResult(400, "Bad Request", QStringLiteral("invalid_json")));
        return;
    }

    // /v2/library/{libraryId}/{comic|folder}/{itemId}
    const QString path = QUrl::fromPercentEncoding(request.getPath());
    const QStringList pathElements = path.split('/');
    const int legacyLibraryId = pathElements.value(3).toInt();
    const QString itemType = pathElements.value(4);
    const qulonglong itemId = pathElements.value(5).toULongLong();

    const auto libraries = DBHelper::getLibraries();
    const QUuid libraryId = libraries.getLibraryIdFromLegacyId(legacyLibraryId);

    UpdateResult result;
    QString connectionName;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(libraries.getDBPath(legacyLibraryId));
        if (!db.isOpen()) {
            result = databaseError();
        } else if (itemType == "comic") {
            result = updateComic(db, legacyLibraryId, libraryId, itemId, json.object());
        } else {
            result = updateFolder(db, legacyLibraryId, libraryId, itemId, json.object());
        }
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (result.status == 200) {
        changedLibraryId = libraryId;
    }

    writeResult(response, result);
}
