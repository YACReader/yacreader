#ifndef COMICS_REMOVER_H
#define COMICS_REMOVER_H

#include <QThread>

#include <QModelIndex>
#include <comic_db.h>

class ComicsRemover : public QObject
{
    Q_OBJECT
public:
    explicit ComicsRemover(QModelIndexList &indexList, QList<QString> &paths, qulonglong parentId, QObject *parent = 0);

signals:
    void remove(int);
    void removeError();
    void finished();
    void removedItemsFromFolder(qulonglong);

public slots:
    void process();

private:
    QModelIndexList indexList;
    QList<QString> paths;
    qulonglong parentId;
};

class FoldersRemover : public QObject
{
    Q_OBJECT
public:
    explicit FoldersRemover(QModelIndexList &indexList, QList<QString> &paths, QObject *parent = 0);

signals:
    void remove(QModelIndex);
    void removeError();
    void finished();

public slots:
    void process();

private:
    QModelIndexList indexList;
    QList<QString> paths;
};

#endif // COMICS_REMOVER_H
