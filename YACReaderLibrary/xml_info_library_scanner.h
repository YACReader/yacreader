#ifndef XMLINFOLIBRARYSCANNER_H
#define XMLINFOLIBRARYSCANNER_H

#include "data_base_management.h"

#include <QSqlQuery>
#include <QtCore>

#include <atomic>

namespace YACReader {

class XMLInfoLibraryScanner : public QThread
{
    Q_OBJECT
public:
    XMLInfoLibraryScanner();
    void scanLibrary(const QString &source, const QString &target);
    void scanFolder(const QString &source, const QString &target, const QString &folder, const QModelIndex &dest);
    QString errorString() const { return scanError; }
    LibraryMaintenanceLockFailure maintenanceFailure() const { return lockFailure; }
    bool completedSuccessfully() const { return completed; }
    bool wasCanceled() const { return cancelRequested; }

protected:
    void run() override;

public slots:
    void stop();
    void cancel();

signals:
    void comicScanned(QString, QString);
    void failed(const QString &error);

private:
    QString source;
    QString target;
    std::atomic_bool stopRunning { false };
    std::atomic_bool cancelRequested { false };
    bool partialUpdate;
    QString folderPath;
    QString scanError;
    LibraryMaintenanceLockFailure lockFailure;
    bool completed { false };

    bool updateFromSQLQuery(QSqlDatabase &db, QSqlQuery &query);
};

}

#endif // XMLINFOLIBRARYSCANNER_H
