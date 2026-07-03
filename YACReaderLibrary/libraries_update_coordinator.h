
#ifndef LIBRARIES_UPDATE_COORDINATOR_H
#define LIBRARIES_UPDATE_COORDINATOR_H

#include <QtCore>

class YACReaderLibraries;
class LibraryCreator;

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

    void init();
    bool isRunning() const;
    UpdateRequestResult requestLibrariesUpdate();
    UpdateRequestResult requestSingleLibraryUpdate(int id);

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
    void updateLibrary(const QString &path);

    QSettings *settings;
    YACReaderLibraries &libraries;
    QTimer timer;
    QElapsedTimer elapsedTimer;
    std::future<void> updateFuture;
    mutable QMutex futureMutex;
    bool canceled;
    std::weak_ptr<LibraryCreator> currentLibraryCreator;

    std::function<bool()> canStartUpdateProvider;
};

#endif // LIBRARIES_UPDATE_COORDINATOR_H
