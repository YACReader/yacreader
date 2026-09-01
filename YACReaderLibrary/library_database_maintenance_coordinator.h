#ifndef LIBRARY_DATABASE_MAINTENANCE_COORDINATOR_H
#define LIBRARY_DATABASE_MAINTENANCE_COORDINATOR_H

#include <QObject>
#include <QString>

#include <functional>

class QWidget;
class YACReaderLibraries;

class LibraryDatabaseMaintenanceCoordinator : public QObject
{
    Q_OBJECT

public:
    using CurrentLibraryNameProvider = std::function<QString()>;

    LibraryDatabaseMaintenanceCoordinator(YACReaderLibraries &libraries, QWidget *dialogParent, CurrentLibraryNameProvider currentLibraryNameProvider);

    void backupCurrentLibrary(const QString &dialogTitle);
    void restoreCurrentLibrary(const QString &dialogTitle);
    void offerDatabaseRecovery(const QString &libraryName, const QString &restoreDialogTitle);

signals:
    void backupAvailabilityChanged(bool available);
    void maintenanceStarted();
    void libraryReloadRequested(const QString &libraryName);
    void libraryUpdateRequested();
    void invalidDatabaseRestoreCancelled();
    void databaseUnavailableAfterRestore();
    void databaseSalvageFailed();

private:
    void backupLibrary(const QString &libraryPath, const QString &dialogTitle);
    void restoreLibrary(const QString &libraryName, const QString &libraryPath, const QString &dialogTitle);
    void startLibraryRestore(const QString &libraryName, const QString &libraryPath, const QString &backupPath, const QString &dialogTitle, bool allowInvalidCurrent = false, bool removeStaleLock = false);
    void startDatabaseSalvage(const QString &libraryName, const QString &libraryPath, bool removeStaleLock = false);

    YACReaderLibraries &libraries;
    QWidget *dialogParent;
    CurrentLibraryNameProvider currentLibraryNameProvider;
};

#endif
