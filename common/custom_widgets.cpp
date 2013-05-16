#include "custom_widgets.h"

#include <QDir>
#include <QFileInfo>

void delTree(QDir dir)
{
	dir.setFilter(QDir::AllDirs|QDir::Files|QDir::Hidden|QDir::NoDotAndDotDot);
	QFileInfoList list = dir.entryInfoList();
	for (int i = 0; i < list.size(); ++i)
	{
		QFileInfo fileInfo = list.at(i);
		QString path = fileInfo.filePath();
		if(fileInfo.isDir())
		{
			delTree(QDir(fileInfo.absoluteFilePath()));
			dir.rmdir(fileInfo.absoluteFilePath());
		}
		else
		{
			dir.remove(fileInfo.absoluteFilePath());
		}
	}
}
