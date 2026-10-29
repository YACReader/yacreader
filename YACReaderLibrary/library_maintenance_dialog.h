#ifndef LIBRARY_MAINTENANCE_DIALOG_H
#define LIBRARY_MAINTENANCE_DIALOG_H

#include "data_base_management.h"

#include <QCoreApplication>
#include <QMessageBox>
#include <QPushButton>

// The caller retains the acquired lock for its entire protected operation.
inline bool acquireLibraryMaintenanceWithRecovery(LibraryMaintenanceLock &lock, const QString &libraryPath, QWidget *parent, const QString &title)
{
    if (!lock.tryLock()) {
        if (lock.error() != QLockFile::LockFailedError) {
            QMessageBox::critical(parent, title, lock.errorString());
            return false;
        }
        const bool running = lock.holderIsRunning();
        auto text = running
                ? QCoreApplication::translate("LibraryMaintenanceDialog", "This library is busy. Wait for maintenance to finish. If it appears stuck, close the application shown in Details, then retry.")
                : QCoreApplication::translate("LibraryMaintenanceDialog", "An earlier maintenance operation may not have released this library. Before recovering access, make sure no YACReaderLibrary or YACReaderLibraryServer operation is using it, including on another computer.");
        QMessageBox message(QMessageBox::Warning, title, text, QMessageBox::Cancel, parent);
        message.setDetailedText(lock.errorString());
        auto retry = message.addButton(running
                                               ? QCoreApplication::translate("LibraryMaintenanceDialog", "Retry")
                                               : QCoreApplication::translate("LibraryMaintenanceDialog", "Recover and retry"),
                                       QMessageBox::AcceptRole);
        message.setDefaultButton(QMessageBox::Cancel);
        message.exec();
        if (message.clickedButton() != retry)
            return false;
        // Reuse the inspected lock so recovery rejects a changed owner.
        if (!lock.tryLock(!running)) {
            QMessageBox::critical(parent, title, lock.errorString());
            return false;
        }
    }
    QString error;
    if (!DataBaseManagement::recoverInterruptedRestore(libraryPath, &error, true)) {
        QMessageBox::critical(parent, title, error);
        return false;
    }
    return true;
}

inline bool recoverLibraryMaintenance(QWidget *parent, const QString &libraryPath, const QString &title)
{
    LibraryMaintenanceLock lock(libraryPath);
    return acquireLibraryMaintenanceWithRecovery(lock, libraryPath, parent, title);
}

#endif
