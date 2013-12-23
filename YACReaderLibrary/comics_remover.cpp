#include "comics_remover.h"

#include <QFile>

ComicsRemover::ComicsRemover(QModelIndexList & il, QList<QString> &  ps, QObject *parent) :
	QThread(parent),indexList(il), paths(ps)
{
}

void ComicsRemover::run()
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
}
