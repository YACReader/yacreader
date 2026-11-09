
#ifndef LIBRARIES_UPDATE_COORDINATOR_H
#define LIBRARIES_UPDATE_COORDINATOR_H

#include "data_base_management.h"

#include <QtCore>

#include <atomic>

class YACReaderLibraries;
class LibraryCreator;
namespace YACReader {
class XMLInfoLibraryScanner;
}

class LibrariesUpdateCoordinator : public QObject
{
    Q_OBJECT
public:
    enum class UpdateRequestResult {
        Started,
        AlreadyRunning,
        NotAllowed,
        LibraryNotFound
    };

    LibrariesUpdateCoordinator(QSettings *settings, YACReaderLibraries &libraries, const std::function<bool()> &canStartUpdateProvider, QObject *parent = 0);
    ~LibrariesUpdateCoordinator() override;

    enum class RecoveryResult {
        Recovered,
        AlreadyRunning,
        NotAllowed,
        LibraryNotFound,
        NotRecoverable,
        Failed
    };

    void init();
    bool isRunning() const;
    UpdateRequestResult requestLibrariesUpdate();
    UpdateRequestResult requestSingleLibraryUpdate(int id);
    UpdateRequestResult requestSingleLibraryXmlRescan(int id);
    // updates and XML rescans that could not take the maintenance lock
    QJsonArray lockProblems() const;
    RecoveryResult recoverLibraryLock(int id, const QString &recoveryToken, QString *error = nullptr);

public slots:
    void updateLibraries();
    void updateSingleLibrary(int id);
    void stop();
    void cancel();

signals:
    void updateStarted();
    void updateEnded();

private slots:
    void checkUpdatePolicy();

private:
    UpdateRequestResult startUpdate(const QStringList &paths);
    UpdateRequestResult startXmlRescan(const QString &path);
    void updateLibrary(const QString &path);
    void rescanLibraryXml(const QString &path);
    void clearLockProblem(const QString &path);
    void recordLockProblem(const QString &path, const QString &operation, const LibraryMaintenanceLockFailure &lockFailure);
    void recordFailure(const QString &path, const QString &operation, const QString &error, const LibraryMaintenanceLockFailure &lockFailure);

    QSettings *settings;
    YACReaderLibraries &libraries;
    QTimer timer;
    QElapsedTimer elapsedTimer;
    std::future<void> updateFuture;
    mutable QMutex futureMutex;
    std::atomic_bool canceled { false };
    mutable QMutex problemsMutex;
    QHash<QString, QJsonObject> problems;
    QMutex workersMutex;
    std::weak_ptr<LibraryCreator> currentLibraryCreator;
    std::weak_ptr<YACReader::XMLInfoLibraryScanner> currentXmlInfoLibraryScanner;

    std::function<bool()> canStartUpdateProvider;
};

#endif // LIBRARIES_UPDATE_COORDINATOR_H
