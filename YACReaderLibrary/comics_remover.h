#ifndef COMICS_REMOVER_H
#define COMICS_REMOVER_H

#include <QThread>

#include <QModelIndex>
#include <comic_db.h>

class ComicsRemover : public QThread
{
    Q_OBJECT
public:
    explicit ComicsRemover(QModelIndexList & indexList, QList<QString> & paths, QObject *parent = 0);
    
signals:
   void remove(int);
   void finished();

private:
    void run();

private:
	QModelIndexList indexList;
	QList<QString> paths;
};

#endif // COMICS_REMOVER_H
