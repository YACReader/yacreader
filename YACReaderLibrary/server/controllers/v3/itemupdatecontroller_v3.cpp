#include "itemupdatecontroller_v3.h"

#include "comic_db.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "folder.h"
#include "overloaded.h"
#include "yacreader_libraries.h"
#include "yacreader_server_data_helper.h"

#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlDatabase>

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
    Rating,
};

// The fields that clients can change. The names are the same as in the item JSON.
const QHash<QString, FieldType> &comicFields()
{
    static const QHash<QString, FieldType> fields {
        { QStringLiteral("read"), FieldType::Bool },
        { QStringLiteral("file_type"), FieldType::FileType },
        { QStringLiteral("rating"), FieldType::Rating },
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

bool isIntegerInRange(const QJsonValue &value, int min, int max)
{
    if (!value.isDouble()) {
        return false;
    }

    const auto number = value.toDouble();
    const auto integer = value.toInt(min - 1);
    return number == integer && integer >= min && integer <= max;
}

bool isValidValue(FieldType type, const QJsonValue &value)
{
    switch (type) {
    case FieldType::Bool:
        return value.isBool();
    case FieldType::FileType:
        return isIntegerInRange(value, static_cast<int>(YACReader::FileType::Comic), static_cast<int>(YACReader::FileType::Yonkoma));
    case FieldType::Rating:
        return isIntegerInRange(value, 0, 5);
    }
    return false;
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

        if (!isValidValue(field.value(), it.value())) {
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

UpdateResult updateComic(QSqlDatabase &db, int libraryId, const QUuid &libraryUuid, ComicId comicId, const QJsonObject &json)
{
    if (auto error = validate(json, comicFields())) {
        return error.value();
    }

    bool found = false;
    DBHelper::loadComic(comicId.value(), db, found);
    if (!found) {
        return errorResult(404, "Not Found", QStringLiteral("comic_not_found"));
    }

    const bool success = runInTransaction(db, [&] {
        if (json.contains("read") && !DBHelper::setComicRead(comicId.value(), json.value("read").toBool(), db)) {
            return false;
        }
        if (json.contains("file_type") && !DBHelper::setComicType(comicId.value(), fileType(json.value("file_type")), db)) {
            return false;
        }
        if (json.contains("rating") && !DBHelper::setComicRating(comicId.value(), json.value("rating").toInt(), db)) {
            return false;
        }
        return true;
    });

    if (!success) {
        return databaseError();
    }

    const auto comic = DBHelper::loadComic(comicId.value(), db, found);
    return { 200, "OK", QJsonDocument(YACReaderServerDataHelper::fullComicToJSON(libraryId, libraryUuid, comic)) };
}

UpdateResult updateFolder(QSqlDatabase &db, int libraryId, const QUuid &libraryUuid, FolderId folderId, const QJsonObject &json)
{
    if (auto error = validate(json, folderFields())) {
        return error.value();
    }

    if (!DBHelper::loadFolder(folderId.value(), db)) {
        return errorResult(404, "Not Found", QStringLiteral("folder_not_found"));
    }

    const bool success = runInTransaction(db, [&] {
        if (json.contains("finished") && !DBHelper::setFolderFinished(folderId.value(), json.value("finished").toBool(), db)) {
            return false;
        }
        if (json.contains("completed") && !DBHelper::setFolderCompleted(folderId.value(), json.value("completed").toBool(), db)) {
            return false;
        }
        // the type applies to the folder and all its content, as in YACReaderLibrary
        if (json.contains("file_type") && !DBHelper::updateFolderTreeType(folderId.value(), db, fileType(json.value("file_type")))) {
            return false;
        }
        return true;
    });

    if (!success) {
        return databaseError();
    }

    const auto folder = DBHelper::loadFolder(folderId.value(), db);
    if (!folder) {
        return errorResult(404, "Not Found", QStringLiteral("folder_not_found"));
    }
    return { 200, "OK", QJsonDocument(YACReaderServerDataHelper::folderToJSON(libraryId, libraryUuid, *folder)) };
}

} // namespace

ItemUpdateControllerV3::ItemUpdateControllerV3(int libraryId, UpdatedItemId itemId)
    : libraryId(libraryId), itemId(itemId)
{
}

void ItemUpdateControllerV3::service(HttpRequest &request, HttpResponse &response)
{
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

    const auto libraries = DBHelper::getLibraries();
    const QUuid libraryUuid = libraries.getLibraryIdFromLegacyId(libraryId);

    UpdateResult result;
    QString connectionName;
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(libraries.getDBPath(libraryId));
        if (!db.isOpen()) {
            result = databaseError();
        } else {
            result = std::visit(overloaded {
                                        [&](ComicId id) { return updateComic(db, libraryId, libraryUuid, id, json.object()); },
                                        [&](FolderId id) { return updateFolder(db, libraryId, libraryUuid, id, json.object()); },
                                },
                                itemId);
        }
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (result.status == 200) {
        changedLibraryId = libraryUuid;
    }

    writeResult(response, result);
}
