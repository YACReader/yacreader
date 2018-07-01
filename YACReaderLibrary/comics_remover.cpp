#include "comics_remover.h"

#include <QFile>
#include <QDir>

#include "QsLog.h"

ComicsRemover::ComicsRemover(QModelIndexList & il, QList<QString> &  ps, qulonglong parentId, QObject *parent)
    :QObject(parent),indexList(il), paths(ps), parentId(parentId)
{
}

void ComicsRemover::process()
{
	QString currentComicPath;
	QListIterator<QModelIndex> i(indexList);
	QListIterator<QString> i2(paths);
	i.toBack();
	i2.toBack();

	while (i.hasPrevious() && i2.hasPrevious())
	{
		QModelIndex mi = i.previous(); 
		currentComicPath = i2.previous();
		if(QFile::remove(currentComicPath))
			emit remove(mi.row());
		else
			emit removeError();
	}

	emit finished();
    emit removedItemsFromFolder(parentId);
}


FoldersRemover::FoldersRemover(QModelIndexList &il, QList<QString> &ps, QObject *parent)
    :QObject(parent),indexList(il), paths(ps)
{

}

void FoldersRemover::process()
{
    QString currentFolderPath;
    QListIterator<QModelIndex> i(indexList);
    QListIterator<QString> i2(paths);
    i.toBack();
    i2.toBack();

    QLOG_DEBUG() << "Deleting folders" << paths.at(0);

    while (i.hasPrevious() && i2.hasPrevious())
    {
        QModelIndex mi = i.previous();
        currentFolderPath = i2.previous();
        QDir d(currentFolderPath);
        if(d.removeRecursively() || !d.exists()) //the folder is in the DB but no in the drive...
            emit remove(mi);
        else
            emit removeError();
    }

    emit finished();
}
