
#ifndef LIBRARIES_UPDATE_COORDINATOR_H
#define LIBRARIES_UPDATE_COORDINATOR_H

#include <QtCore>

class YACReaderLibraries;

class LibrariesUpdateCoordinator : public QObject
{
    Q_OBJECT
public:
    LibrariesUpdateCoordinator(QSettings *settings, YACReaderLibraries &libraries, QObject *parent = 0);

    void updateLibraries();

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
    QTimer *timer;
    QElapsedTimer elapsedTimer;
    std::future<void> updateFuture;
};

#endif // LIBRARIES_UPDATE_COORDINATOR_H
