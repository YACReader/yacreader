
#ifndef LIBRARIES_UPDATE_COORDINATOR_H
#define LIBRARIES_UPDATE_COORDINATOR_H

#include <QtCore>

class YACReaderLibraries;
class LibraryCreator;

class LibrariesUpdateCoordinator : public QObject
{
    Q_OBJECT
public:
    LibrariesUpdateCoordinator(QSettings *settings, YACReaderLibraries &libraries, const std::function<bool()> &canStartUpdateProvider, QObject *parent = 0);

    void init();
    void updateLibraries();
    bool isRunning() const;

public slots:
    void stop();
    void cancel();

signals:
    void updateStarted();
    void updateEnded();

private slots:
    void checkUpdatePolicy();
    void startUpdate();
    void updateLibrary(const QString &path);

private:
    QSettings *settings;
    YACReaderLibraries &libraries;
    QTimer timer;
    QElapsedTimer elapsedTimer;
    std::future<void> updateFuture;
    bool canceled;
    std::weak_ptr<LibraryCreator> currentLibraryCreator;

    std::function<bool()> canStartUpdateProvider;
};

#endif // LIBRARIES_UPDATE_COORDINATOR_H
