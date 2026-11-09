#ifndef COMIC_INFO_REPAIRER_H
#define COMIC_INFO_REPAIRER_H

#include <QLockFile>
#include <QStringList>
#include <QThread>

#include <atomic>

class QSettings;

namespace YACReader {

struct ComicInfoRepairSummary {
    int repaired { 0 };
    int failed { 0 };
    int missingFiles { 0 };
    bool canceled { false };
    QString error;
    QStringList failedFilePaths;
    QStringList missingFilePaths;
    // set on lock contention with another maintenance operation; holder info comes from the
    // lock file and is empty if it couldn't be read
    bool lockedByAnotherProcess { false };
    QLockFile::LockError maintenanceError { QLockFile::NoError };
    bool lockHolderIsRunningLocally { false };
    QString lockFilePath;
    QString lockHolderInfo;
};

class ComicInfoRepairer : public QThread
{
    Q_OBJECT

public:
    explicit ComicInfoRepairer(QSettings *settings, QObject *parent = nullptr);

    // removeStaleLock attempts removal only after lock contention, for retrying
    // after the user confirmed that the previous holder is not running anymore
    void repairLibrary(const QString &source, const QString &target, bool removeStaleLock = false);
    ComicInfoRepairSummary summary() const;

public slots:
    void stop();

signals:
    void comicProcessed(const QString &relativePath, const QString &coverPath);
    void failed(const QString &error);

protected:
    void run() override;

private:
    QSettings *settings;
    QString source;
    QString target;
    bool importXmlMetadata { false };
    bool removeStaleLock { false };
    std::atomic_bool stopRequested { false };
    ComicInfoRepairSummary repairSummary;
};

}

#endif // COMIC_INFO_REPAIRER_H
