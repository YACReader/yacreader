#ifndef ORGANIZE_FILES_PLAN_H
#define ORGANIZE_FILES_PLAN_H

#include <QHash>
#include <QList>
#include <QMetaType>
#include <QPair>
#include <QSet>
#include <QString>
#include <QStringList>

namespace OrganizeFiles {

// Rename keeps every comic in its own folder and only changes the file name.
// Organize may move files and create folders.
enum class Mode {
    Rename,
    Organize
};

struct ComicEntry {
    qulonglong comicId = 0;
    QString sourceAbsolute;
    QString baseName;
    QString extension;
    QString folderRelative;
    bool missing = false;

    QString publisher;
    QString imprint;
    QString series;
    QString volume;
    QString number;
    QString count;
    QString title;
    QString year;
    QString month;
    QString storyArc;
    QString arcNumber;
    QString writer;
};

struct PlannedMove {
    enum class Status {
        Move,
        Unchanged,
        Renamed,
        Incomplete,
        Missing,
        Excluded
    };

    qulonglong comicId = 0;
    QString sourceAbsolute;
    QString destinationRelative;
    Status status = Status::Move;
    bool edited = false;
    QString note;
};

struct Override {
    bool excluded = false;
    QString destinationRelative;
};

using Overrides = QHash<QString, Override>;

QStringList knownTokens();
QStringList invalidTokens(const QString &pattern);
bool patternCreatesFolders(const QString &pattern);

// Folds a path the way the local file system compares them. Use it to decide
// whether two paths are the same file, never to decide whether a name changed.
QString pathKey(const QString &path);

QString sanitizeSegment(QString segment);
QString padNumber(const QString &number, int width);

// The database keeps the publication date as one d/M/yyyy string, so the year
// and the month tokens have to be read back out of it.
void applyPublicationDate(ComicEntry &entry, const QString &date);

QString buildRelativePath(const QString &pattern, const ComicEntry &entry, QStringList *fallbackFields = nullptr);

QString defaultPattern(Mode mode);
QList<QPair<QString, QString>> presets(Mode mode);

class PlanBuilder
{
public:
    PlanBuilder(const QList<ComicEntry> &entries, const QString &base, Mode mode);

    void setBase(const QString &base);
    QList<PlannedMove> build(const QString &pattern, const Overrides &overrides);

private:
    const QHash<QString, QString> &namesIn(const QString &absoluteDirectory);
    QString canonicalDirectory(const QString &absoluteDirectory);

    QList<ComicEntry> entries;
    QString base;
    Mode mode;
    QSet<QString> sourcePaths;
    // Per directory: the names it holds on disk, folded key to actual casing.
    QHash<QString, QHash<QString, QString>> directoryNames;
    // Folded path to the casing the run will produce; rebuilt on every build().
    QHash<QString, QString> canonicalDirectories;
};

}

Q_DECLARE_METATYPE(OrganizeFiles::PlannedMove)
Q_DECLARE_METATYPE(QList<OrganizeFiles::PlannedMove>)
Q_DECLARE_METATYPE(OrganizeFiles::Overrides)

#endif // ORGANIZE_FILES_PLAN_H
