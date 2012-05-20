#include "comic.h"

#include <QPixmap>
#include <QRegExp>
#include <QString>
#include <algorithm>
#include <QDir>
#include <QFileInfoList>
#include "bookmarks.h"
#include "qnaturalsorting.h"

#define EXTENSIONS << "*.jpg" << "*.jpeg" << "*.png" << "*.gif" << "*.tiff" << "*.tif" << "*.bmp"

//-----------------------------------------------------------------------------
Comic::Comic()
:_pages(),_index(0),_path(),_loaded(false),bm(new Bookmarks())
{
    setup();
}
//-----------------------------------------------------------------------------
Comic::Comic(const QString pathFile)
:_pages(),_index(0),_path(pathFile),_loaded(false),bm(new Bookmarks())
{
    setup();
    loadFromFile(pathFile);
}
//-----------------------------------------------------------------------------
void Comic::setup()
{
    connect(this,SIGNAL(pageChanged(int)),this,SLOT(checkIsBookmark(int)));
	connect(this,SIGNAL(imageLoaded(int)),this,SLOT(updateBookmarkImage(int)));
}
//-----------------------------------------------------------------------------
void Comic::load(const QString & path)
{
    QFileInfo fi(path);
    bm->newComic(path);
	emit bookmarksLoaded(*bm);
    if(fi.isFile())
    {
	loadFromFile(path);
    }
    else
    {
	if(fi.isDir())
	{
	    loadFromDir(path);
	}
    }
}
//-----------------------------------------------------------------------------
void Comic::loadFromFile(const QString & pathFile)
{
	_path = QDir::cleanPath(pathFile);
	//load files size
	_7z = new QProcess();
	QStringList attributes;
	attributes << "l" << "-ssc-" << "-r" << _path EXTENSIONS;
	connect(_7z,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(loadSizes(void)));
	connect(_7z,SIGNAL(error(QProcess::ProcessError)),this,SLOT(openingError(QProcess::ProcessError)));
	_7z->start(QCoreApplication::applicationDirPath()+"/utils/7z",attributes);
}
//-----------------------------------------------------------------------------
void Comic::loadFromDir(const QString & pathDir)
{
	_pathDir = pathDir;
	start();
}
//-----------------------------------------------------------------------------
void Comic::run()
{
	QDir d(_pathDir);
	QStringList l;
	l EXTENSIONS;
	d.setNameFilters(l);
	d.setFilter(QDir::Files|QDir::NoDotAndDotDot);
	//d.setSorting(QDir::Name|QDir::IgnoreCase|QDir::LocaleAware);
	QFileInfoList list = d.entryInfoList();

	qSort(list.begin(),list.end(),naturalSortLessThanCIFileInfo);

	int nPages = list.size();
	_pages.clear();
	_pages.resize(nPages);
	if(nPages==0)
	{
	    QMessageBox::critical(NULL,tr("No images found"),tr("There are not images on the selected folder"));
	    emit errorOpening();
	}
	else
	{
	emit pageChanged(0); // this indicates new comic, index=0
	emit numPages(_pages.size());
	_loaded = true;
	
	for(int i=0;i<nPages;i++)
	    {
		QFile f(list.at(i).absoluteFilePath());
		f.open(QIODevice::ReadOnly);
		_pages[i]=f.readAll();
		emit imageLoaded(i);
		emit imageLoaded(i,_pages[i]);
	    }
	}
	emit imagesLoaded();
}
//-----------------------------------------------------------------------------
void Comic::loadSizes()
{
	QRegExp rx("[0-9]{4}-[0-9]{2}-[0-9]{2}[ ]+[0-9]{2}:[0-9]{2}:[0-9]{2}[ ]+.{5}[ ]+([0-9]+)[ ]+([0-9]+)[ ]+(.+)");

	QByteArray ba = _7z->readAllStandardOutput();
	QList<QByteArray> lines = ba.split('\n');
	QByteArray line; 
	QString name;
	foreach(line,lines)
	{
		if(rx.indexIn(QString(line))!=-1)
		{
			_sizes.push_back(rx.cap(1).toInt());
			name = rx.cap(3).trimmed();
			_order.push_back(name);
			_fileNames.push_back(name);
		}
	}
	if(_sizes.size()==0)
	{
		QMessageBox::critical(NULL,tr("File error"),tr("File not found or not images in file"));
		emit errorOpening();
		return;
	}
	_pages.resize(_sizes.size());

	emit pageChanged(0); // this indicates new comic, index=0
	emit numPages(_pages.size());
	_loaded = true;

	_cfi=0;
	qSort(_fileNames.begin(),_fileNames.end(), naturalSortLessThanCI);
	int i=0;
	foreach(name,_fileNames)
	{
		_newOrder.insert(name,i);
		i++;
	}
	_7ze = new QProcess();
	QStringList attributes;
	attributes << "e" << "-ssc-" << "-so"  << "-r" << _path EXTENSIONS;
	connect(_7ze,SIGNAL(error(QProcess::ProcessError)),this,SLOT(openingError(QProcess::ProcessError)));
	connect(_7ze,SIGNAL(readyReadStandardOutput()),this,SLOT(loadImages(void)));
	connect(_7ze,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(loadFinished(void)));
	_7ze->start(QCoreApplication::applicationDirPath()+"/utils/7z",attributes);
}
//-----------------------------------------------------------------------------
void Comic::loadImages()
{

	QByteArray ba = _7ze->readAllStandardOutput();
	int s;
	int rigthIndex;
	while(ba.size()>0)
	{
		rigthIndex = _newOrder.value(_order[_cfi]);
		s = _pages[rigthIndex].size();
		_pages[rigthIndex].append(ba.left(_sizes[_cfi]-s));
		ba.remove(0,_sizes[_cfi]-s);
		if(_pages[rigthIndex].size()==static_cast<int>(_sizes[_cfi]))
		{
			emit imageLoaded(rigthIndex);
            emit imageLoaded(rigthIndex,_pages[rigthIndex]);
			_cfi++;
		}
	}
}
//-----------------------------------------------------------------------------
void Comic::openingError(QProcess::ProcessError error)
{
	switch(error)
	{
	case QProcess::FailedToStart:
		QMessageBox::critical(NULL,tr("7z not found"),tr("7z wasn't found in your PATH."));
		break;
	case QProcess::Crashed:
		QMessageBox::critical(NULL,tr("7z crashed"),tr("7z crashed."));
		break;
	case QProcess::ReadError:
		QMessageBox::critical(NULL,tr("7z reading"),tr("problem reading from 7z"));
		break;
	case QProcess::UnknownError:
		QMessageBox::critical(NULL,tr("7z problem"),tr("Unknown error 7z"));
		break;
	default:
		//TODO
		break;
	}	
	_loaded = false;
	emit errorOpening();
}
int Comic::nextPage()
{
	if(_index<_pages.size()-1)
		_index++;

	emit pageChanged(_index);

	return _index;
}
//-----------------------------------------------------------------------------
int Comic::previousPage()
{
	if(_index>0)
		_index--;

	emit pageChanged(_index);

	return _index;
}
//-----------------------------------------------------------------------------
void Comic::setIndex(unsigned int index)
{
	if(static_cast<int>(index)<_pages.size()-1)
		_index = index;
	else
		_index = _pages.size()-1;

	emit pageChanged(_index);
}
//-----------------------------------------------------------------------------
QPixmap * Comic::currentPage()
{
	QPixmap * p = new QPixmap();
	p->loadFromData(_pages[_index]);
	return p;
}
//-----------------------------------------------------------------------------
QPixmap * Comic::operator[](unsigned int index)
{
	QPixmap * p = new QPixmap();
	p->loadFromData(_pages[index]);
	return p;
}
//-----------------------------------------------------------------------------
bool Comic::loaded()
{
	return _loaded;
}
//-----------------------------------------------------------------------------
void Comic::loadFinished()
{
	emit imagesLoaded();
}
//-----------------------------------------------------------------------------
void Comic::setBookmark()
{
    bm->setBookmark(_index,*operator[](_index));
	emit bookmarksLoaded(*bm);
}
//-----------------------------------------------------------------------------
void Comic::removeBookmark()
{
	bm->removeBookmark(_index);
	emit bookmarksLoaded(*bm);
}
//-----------------------------------------------------------------------------
void Comic::saveBookmarks()
{
	bm->setLastPage(_index,*operator[](_index));
	bm->save();
}
//-----------------------------------------------------------------------------
void Comic::checkIsBookmark(int index)
{
    emit isBookmark(bm->isBookmark(index));
}
//-----------------------------------------------------------------------------
void Comic::updateBookmarkImage(int index)
{
	if(bm->isBookmark(index))
	{
		bm->setBookmark(index,*operator[](index));
		emit bookmarksLoaded(*bm);
	}
	if(bm->getLastPage() == index)
	{
		bm->setLastPage(index,*operator[](index));
		emit bookmarksLoaded(*bm);
	}
	
}
