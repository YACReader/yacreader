#ifndef __LIBRARY_CREATOR_H
#define __LIBRARY_CREATOR_H

#include <QObject>
#include <QString>
#include <QDir>
#include <QFile>
#include <QByteArray>
#include <QRegExp>
#include <QProcess>
#include <QtCore>
#include <QtGui>
#include <QMutex>
#include <QThread>
#include <QSqlDatabase>
#include <QModelIndex>

#include "folder.h"
#include "comic_db.h"

class LibraryCreator : public QThread
{
    Q_OBJECT
public:
    LibraryCreator();
    void createLibrary(const QString &source, const QString &target);
    void updateLibrary(const QString &source, const QString &target);
    void updateFolder(const QString &source, const QString &target, const QString &folder, const QModelIndex &dest);
    void stop();

private:
    void processLibrary(const QString &source, const QString &target);
    enum Mode { CREATOR,
                UPDATER };
    // atributos "globales" durante el proceso de creación y actualización
    enum Mode _mode;
    QString _source;
    QString _target;
    QString _sourceFolder; // used for partial updates
    QStringList _nameFilter;
    QString _databaseConnection;
    QList<Folder> _currentPathFolders; // lista de folders en el orden en el que están siendo explorados, el último es el folder actual
    // recursive method
    void create(QDir currentDirectory);
    void update(QDir currentDirectory);
    void run() override;
    qulonglong insertFolders(); // devuelve el id del último folder añadido (último en la ruta)
    bool checkCover(const QString &hash);
    void insertComic(const QString &relativePath, const QFileInfo &fileInfo);
    // qulonglong insertFolder(qulonglong parentId,const Folder & folder);
    // qulonglong insertComic(const Comic & comic);
    bool stopRunning;
    // LibraryCreator está en modo creación si creation == true;
    bool creation;
    bool partialUpdate;
    QModelIndex folderDestinationModelIndex;

signals:
    void finished();
    void coverExtracted(QString);
    void folderUpdated(QString);
    void comicAdded(QString, QString);
    void updated();
    void created();
    void failedCreatingDB(QString);
    void failedOpeningDB(QString);
    void updatedCurrentFolder(QModelIndex);
};

#endif
