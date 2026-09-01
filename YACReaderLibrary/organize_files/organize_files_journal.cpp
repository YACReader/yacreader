#include "organize_files_journal.h"

#include "yacreader_global.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>

namespace OrganizeFiles {

QString absoluteFromRelative(const QString &libraryPath, const QString &relativePath)
{
    return QDir::cleanPath(libraryPath + QLatin1Char('/') + relativePath);
}

Journal::Journal(const QString &libraryPath)
    : libraryPath(QDir::cleanPath(libraryPath))
{
}

QString Journal::directory(const QString &libraryPath)
{
    return QDir(YACReader::LibraryPaths::libraryDataPath(libraryPath)).filePath(QStringLiteral("organize"));
}

QString Journal::filePath() const
{
    return path;
}

bool Journal::healthy() const
{
    return !broken;
}

QString Journal::errorString() const
{
    return error;
}

QString Journal::toRelative(const QString &absolutePath) const
{
    return QLatin1Char('/') + QDir(libraryPath).relativeFilePath(absolutePath);
}

bool Journal::begin(const QString &base)
{
    const QString folder = directory(libraryPath);
    if (!QDir().mkpath(folder)) {
        broken = true;
        error = QCoreApplication::translate("OrganizeFiles", "%1 could not be created").arg(QDir::toNativeSeparators(folder));
        return false;
    }

    path = QDir(folder).filePath(QString::number(QDateTime::currentMSecsSinceEpoch()) + QStringLiteral(".jsonl"));

    file.setFileName(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        broken = true;
        error = file.errorString();
        path.clear();
        return false;
    }

    QJsonObject header;
    header[QStringLiteral("type")] = QStringLiteral("header");
    header[QStringLiteral("version")] = 1;
    header[QStringLiteral("startedAt")] = QDateTime::currentSecsSinceEpoch();
    header[QStringLiteral("base")] = toRelative(base);

    return writeLine(header);
}

bool Journal::reopen(const QString &filePath)
{
    path = filePath;

    file.setFileName(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        broken = true;
        error = file.errorString();
        path.clear();
        return false;
    }

    return true;
}

bool Journal::writeLine(const QJsonObject &object)
{
    if (!file.isOpen()) {
        broken = true;
        return false;
    }

    const QByteArray line = QJsonDocument(object).toJson(QJsonDocument::Compact) + '\n';

    if (file.write(line) != line.size() || !file.flush()) {
        broken = true;
        error = file.errorString();
        return false;
    }

    return true;
}

void Journal::appendMove(qulonglong comicId, const QString &fromAbsolute, const QString &toAbsolute)
{
    QJsonObject move;
    move[QStringLiteral("type")] = QStringLiteral("move");
    move[QStringLiteral("comicId")] = static_cast<qint64>(comicId);
    move[QStringLiteral("from")] = toRelative(fromAbsolute);
    move[QStringLiteral("to")] = toRelative(toAbsolute);
    writeLine(move);
}

void Journal::appendRemovedDirectory(const QString &absolutePath)
{
    QJsonObject removed;
    removed[QStringLiteral("type")] = QStringLiteral("removedDir");
    removed[QStringLiteral("path")] = toRelative(absolutePath);
    writeLine(removed);
}

void Journal::appendCreatedDirectory(const QString &absolutePath)
{
    QJsonObject created;
    created[QStringLiteral("type")] = QStringLiteral("createdDir");
    created[QStringLiteral("path")] = toRelative(absolutePath);
    writeLine(created);
}

void Journal::appendRemovedFolder(const QVariantMap &row)
{
    QJsonObject removed;
    removed[QStringLiteral("type")] = QStringLiteral("removedFolder");
    removed[QStringLiteral("row")] = QJsonObject::fromVariantMap(row);
    writeLine(removed);
}

void Journal::appendCreatedFolder(qulonglong folderId)
{
    QJsonObject created;
    created[QStringLiteral("type")] = QStringLiteral("createdFolder");
    created[QStringLiteral("id")] = static_cast<qint64>(folderId);
    writeLine(created);
}

void Journal::finish()
{
    QJsonObject footer;
    footer[QStringLiteral("type")] = QStringLiteral("footer");
    footer[QStringLiteral("finishedAt")] = QDateTime::currentSecsSinceEpoch();
    footer[QStringLiteral("complete")] = !broken;
    writeLine(footer);

    file.close();
}

QString Journal::latestPath(const QString &libraryPath)
{
    QDir folder(directory(libraryPath));
    const auto entries = folder.entryList({ QStringLiteral("*.jsonl") }, QDir::Files, QDir::Name);

    if (entries.isEmpty())
        return QString();

    return folder.filePath(entries.last());
}

bool Journal::read(const QString &libraryPath, const QString &filePath, JournalData *data)
{
    QFile input(filePath);
    if (!input.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    data->filePath = filePath;
    data->moves.clear();
    data->removedDirectories.clear();
    data->createdDirectories.clear();
    data->removedFolders.clear();
    data->createdFolders.clear();
    data->complete = false;

    while (!input.atEnd()) {
        const QByteArray line = input.readLine().trimmed();
        if (line.isEmpty())
            continue;

        const auto object = QJsonDocument::fromJson(line).object();
        const QString type = object.value(QStringLiteral("type")).toString();

        if (type == QLatin1String("header")) {
            data->startedAt = static_cast<qint64>(object.value(QStringLiteral("startedAt")).toDouble());
            data->base = absoluteFromRelative(libraryPath, object.value(QStringLiteral("base")).toString());
        } else if (type == QLatin1String("move")) {
            JournalMove move;
            move.comicId = static_cast<qulonglong>(object.value(QStringLiteral("comicId")).toDouble());
            move.from = object.value(QStringLiteral("from")).toString();
            move.to = object.value(QStringLiteral("to")).toString();
            data->moves.append(move);
        } else if (type == QLatin1String("removedDir")) {
            data->removedDirectories.append(object.value(QStringLiteral("path")).toString());
        } else if (type == QLatin1String("createdDir")) {
            data->createdDirectories.append(object.value(QStringLiteral("path")).toString());
        } else if (type == QLatin1String("removedFolder")) {
            data->removedFolders.append(object.value(QStringLiteral("row")).toObject().toVariantMap());
        } else if (type == QLatin1String("createdFolder")) {
            data->createdFolders.append(static_cast<qulonglong>(object.value(QStringLiteral("id")).toDouble()));
        } else if (type == QLatin1String("footer")) {
            data->complete = object.value(QStringLiteral("complete")).toBool();
        }
    }

    return true;
}

void Journal::prune(const QString &libraryPath, int keep)
{
    QDir folder(directory(libraryPath));
    const auto entries = folder.entryList({ QStringLiteral("*.jsonl") }, QDir::Files, QDir::Name);

    for (int i = 0; i < entries.size() - keep; ++i)
        QFile::remove(folder.filePath(entries.at(i)));
}

}
