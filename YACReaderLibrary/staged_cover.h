#ifndef STAGED_COVER_H
#define STAGED_COVER_H

#include <QImage>
#include <QString>

// a new cover file that replaces the cover at `coverPath` only together with a database commit:
// stage it, write the database, install() before the commit, then finish() after it, or restore() on any failure
// the cover path is never empty, so a crash leaves only a staged file or a copy of the previous cover
class StagedCover
{
public:
    explicit StagedCover(const QString &coverPath);

    // where the new cover is written; an extractor can write there directly
    QString stagedPath() const;
    bool stage(const QImage &image);
    bool isStaged() const;

    // the staged file becomes the cover; the previous cover is kept until finish() or restore()
    bool install();
    void finish();
    // the previous cover comes back and the staged file is removed
    void restore();

    // removes what an interrupted StagedCover left in `coversPath`; a previous cover comes back if its cover is missing
    static void removeLeftovers(const QString &coversPath);

private:
    QString coverPath;
    QString backupPath;
    bool installed = false;
};

#endif
