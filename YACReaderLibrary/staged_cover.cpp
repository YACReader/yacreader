#include "staged_cover.h"

#include "cover_utils.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>

#include <filesystem>
#include <system_error>

namespace {

constexpr auto stagedSuffix = ".new.jpg";
constexpr auto backupSuffix = ".old";

// QFile::rename refuses an existing target; this replaces it in one step
bool replaceFile(const QString &source, const QString &target)
{
    std::error_code error;
    std::filesystem::rename(QFileInfo(source).filesystemFilePath(), QFileInfo(target).filesystemFilePath(), error);
    return !error;
}

}

StagedCover::StagedCover(const QString &coverPath)
    : coverPath(coverPath)
{
}

QString StagedCover::stagedPath() const
{
    return coverPath + stagedSuffix;
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
        backupPath = coverPath + backupSuffix;
        QFile::remove(backupPath);
        if (!QFile::copy(coverPath, backupPath)) {
            QFile::remove(backupPath);
            backupPath.clear();
            return false;
        }
    }

    if (!replaceFile(stagedPath(), coverPath)) {
        if (!backupPath.isEmpty())
            QFile::remove(backupPath);
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
        if (backupPath.isEmpty())
            QFile::remove(coverPath);
        else
            replaceFile(backupPath, coverPath);
    }
    QFile::remove(stagedPath());
    backupPath.clear();
    installed = false;
}

void StagedCover::removeLeftovers(const QString &coversPath)
{
    QDirIterator leftovers(coversPath, { QString("*") + stagedSuffix, QString("*") + backupSuffix }, QDir::Files);
    while (leftovers.hasNext()) {
        const auto path = leftovers.next();
        if (path.endsWith(backupSuffix)) {
            const auto cover = path.chopped(qstrlen(backupSuffix));
            if (QFile::exists(cover))
                QFile::remove(path);
            else
                QFile::rename(path, cover);
        } else {
            QFile::remove(path);
        }
    }
}
