#ifndef XMLINFOLIBRARYSCANNER_H
#define XMLINFOLIBRARYSCANNER_H

#include <QtCore>
#include <QSqlQuery>

namespace YACReader {

class XMLInfoLibraryScanner : public QThread
{
    Q_OBJECT
public:
    XMLInfoLibraryScanner();
    void scanLibrary(const QString &source, const QString &target);
    void scanFolder(const QString &source, const QString &target, const QString &folder, const QModelIndex &dest);

protected:
    void run() override;

public slots:
    void stop();

signals:
    void comicScanned(QString, QString);

private:
    QString source;
    QString target;
    bool stopRunning;
    bool partialUpdate;
    QModelIndex folderDestinationModelIndex;

    void updateFromSQLQuery(QSqlDatabase &db, QSqlQuery &query);
};

}

#endif // XMLINFOLIBRARYSCANNER_H
