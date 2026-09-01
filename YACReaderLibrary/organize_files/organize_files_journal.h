#ifndef ORGANIZE_FILES_JOURNAL_H
#define ORGANIZE_FILES_JOURNAL_H

#include <QFile>
#include <QList>
#include <QString>
#include <QStringList>
#include <QVariantMap>

class QJsonObject;

namespace OrganizeFiles {

struct JournalMove {
    qulonglong comicId = 0;
    QString from;
    QString to;
};

struct JournalData {
    QString filePath;
    QString base;
    qint64 startedAt = 0;
    bool complete = false;
    QList<JournalMove> moves;
    QStringList removedDirectories;
    // Undo may remove these and nothing else; a pre-existing directory is not the
    // run's to delete.
    QStringList createdDirectories;
    // Full rows, so undo can restore them with the same ids (covers are keyed by id).
    QList<QVariantMap> removedFolders;
    // Parents before children; undo deletes the ones that are empty again.
    QList<qulonglong> createdFolders;
};

class Journal
{
public:
    explicit Journal(const QString &libraryPath);

    bool begin(const QString &base);
    // Reopens to append: the database work runs after the moves and has to land in
    // the same record.
    bool reopen(const QString &filePath);

    void appendMove(qulonglong comicId, const QString &fromAbsolute, const QString &toAbsolute);
    void appendRemovedDirectory(const QString &absolutePath);
    void appendCreatedDirectory(const QString &absolutePath);
    void appendRemovedFolder(const QVariantMap &row);
    void appendCreatedFolder(qulonglong folderId);
    void finish();

    QString filePath() const;
    // False once a line failed to reach the disk; the caller must stop moving files.
    bool healthy() const;
    QString errorString() const;

    static QString directory(const QString &libraryPath);
    static QString latestPath(const QString &libraryPath);
    static bool read(const QString &libraryPath, const QString &filePath, JournalData *data);
    static void prune(const QString &libraryPath, int keep);

private:
    bool writeLine(const QJsonObject &object);
    QString toRelative(const QString &absolutePath) const;

    QString libraryPath;
    QString path;
    QFile file;
    bool broken = false;
    QString error;
};

QString absoluteFromRelative(const QString &libraryPath, const QString &relativePath);

}

#endif // ORGANIZE_FILES_JOURNAL_H
