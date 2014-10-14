#ifndef COMIC_FILES_MANAGER_H
#define COMIC_FILES_MANAGER_H

#include <QObject>

//this class is intended to work in background, just use moveToThread and process to start working
class ComicFilesManager : public QObject
{
    Q_OBJECT
public:
    explicit ComicFilesManager(QObject *parent = 0);
    void copyComicsTo(const QList<QPair<QString,QString> > & sourceComics, const QString & folderDest);
    void moveComicsTo(const QList<QPair<QString,QString> > & comics, const QString & folderDest);
    static QList<QPair<QString, QString> > getDroppedFiles(const QList<QUrl> & urls);
signals:
    void currentComic(QString);
    void progress(int);
    void finished();
    void success(); //at least one comics has been copied or moved
public slots:
    void process();
    void cancel();

protected:
    bool move;
    bool canceled;
    QList<QPair<QString,QString> > comics;
    QString folder;

};

#endif // COMIC_FILES_MANAGER_H
