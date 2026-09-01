#ifndef LIBRARY_REPAIR_COORDINATOR_H
#define LIBRARY_REPAIR_COORDINATOR_H

#include <QObject>
#include <QString>

#include <functional>

class QSettings;
class QWidget;
class YACReaderLibraries;

namespace YACReader {
class ComicInfoRepairer;
}

class LibraryRepairCoordinator : public QObject
{
    Q_OBJECT

public:
    using CurrentLibraryNameProvider = std::function<QString()>;

    LibraryRepairCoordinator(QSettings *settings, YACReaderLibraries &libraries, QWidget *dialogParent, CurrentLibraryNameProvider currentLibraryNameProvider);

    void repairCurrentLibrary(const QString &dialogTitle);
    void stop();

signals:
    void repairStarted();
    void repairFinished();
    void comicProcessed(const QString &relativePath, const QString &coverPath);
    void databaseRecoveryRequested(const QString &libraryName);

private:
    void startRepair(bool removeStaleLock);
    void handleFinished();
    void handleFailure(const QString &error);

    YACReaderLibraries &libraries;
    QWidget *dialogParent;
    CurrentLibraryNameProvider currentLibraryNameProvider;
    YACReader::ComicInfoRepairer *repairer;
    QString libraryName;
    QString libraryPath;
    QString dialogTitle;
};

#endif
