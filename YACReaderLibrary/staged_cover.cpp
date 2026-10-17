#include "staged_cover.h"

#include "cover_utils.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

StagedCover::StagedCover(const QString &coverPath)
    : coverPath(coverPath)
{
}

QString StagedCover::stagedPath() const
{
    return coverPath + ".new.jpg";
}

bool StagedCover::stage(const QImage &image)
{
    if (!QDir().mkpath(QFileInfo(coverPath).absolutePath()) || !YACReader::saveCover(stagedPath(), image)) {
        QFile::remove(stagedPath());
        return false;
    }
    return true;
}

bool StagedCover::isStaged() const
{
    return QFile::exists(stagedPath());
}

bool StagedCover::install()
{
    if (!isStaged())
        return false;

    if (QFile::exists(coverPath)) {
        backupPath = coverPath + ".old";
        QFile::remove(backupPath);
        if (!QFile::rename(coverPath, backupPath)) {
            backupPath.clear();
            return false;
        }
    }

    if (!QFile::rename(stagedPath(), coverPath)) {
        if (!backupPath.isEmpty())
            QFile::rename(backupPath, coverPath);
        backupPath.clear();
        return false;
    }

    installed = true;
    return true;
}

void StagedCover::finish()
{
    if (!backupPath.isEmpty())
        QFile::remove(backupPath);
    backupPath.clear();
    installed = false;
}

void StagedCover::restore()
{
    if (installed) {
        QFile::remove(coverPath);
        if (!backupPath.isEmpty())
            QFile::rename(backupPath, coverPath);
    }
    QFile::remove(stagedPath());
    backupPath.clear();
    installed = false;
}
