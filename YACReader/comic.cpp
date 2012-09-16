#include "comic.h"

#include <QPixmap>
#include <QRegExp>
#include <QString>
#include <algorithm>
#include <QDir>
#include <QFileInfoList>
#include "bookmarks.h" //TODO desacoplar la dependencia con bookmarks
#include "qnaturalsorting.h"

#define EXTENSIONS << "*.jpg" << "*.jpeg" << "*.png" << "*.gif" << "*.tiff" << "*.tif" << "*.bmp"

//-----------------------------------------------------------------------------
Comic::Comic()
:_pages(),_index(0),_path(),_loaded(false),bm(new Bookmarks()),_loadedPages()
{
    setup();
}
//-----------------------------------------------------------------------------
Comic::Comic(const QString pathFile)
:_pages(),_index(0),_path(pathFile),_loaded(false),bm(new Bookmarks()),_loadedPages()
{
    setup();
    loadFromFile(pathFile);
}
//-----------------------------------------------------------------------------
Comic::~Comic()
{
	QThread::~QThread();
	/*delete _7z;
	delete _7ze;
	delete bm;*/ //TODO safe delete
}
//-----------------------------------------------------------------------------
void Comic::setup()
{
    connect(this,SIGNAL(pageChanged(int)),this,SLOT(checkIsBookmark(int)));
	connect(this,SIGNAL(imageLoaded(int)),this,SLOT(updateBookmarkImage(int)));
	connect(this,SIGNAL(imageLoaded(int)),this,SLOT(setPageLoaded(int)));
}
//-----------------------------------------------------------------------------
bool Comic::load(const QString & path)
{
	QFileInfo fi(path);
	if(fi.exists())
	{
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
		return true;
	}
	else
	{
		QMessageBox::critical(NULL,tr("Not found"),tr("Comic not found"));
		emit errorOpening();
		return false;
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
	_loadedPages = QVector<bool>(nPages,false);
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
	_loadedPages = QVector<bool>(_sizes.size(),false);

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
//-----------------------------------------------------------------------------
void Comic::setPageLoaded(int page)
{
	_loadedPages[page] = true;
}
//-----------------------------------------------------------------------------
QByteArray Comic::getRawPage(int page)
{
	if(page < 0 || page >= _pages.size())
		return QByteArray();
	return _pages[page];
}
//-----------------------------------------------------------------------------
bool Comic::pageIsLoaded(int page)
{
	if(page < 0 || page >= _pages.size())
		return false;
	return _loadedPages[page];
}

//
//
//
//
//
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////




//-----------------------------------------------------------------------------
Comic2::Comic2()
:_pages(),_index(0),_path(),_loaded(false),_loadedPages()
{
    setup();
}
//-----------------------------------------------------------------------------
Comic2::Comic2(const QString pathFile)
:_pages(),_index(0),_path(pathFile),_loaded(false),_loadedPages()
{
    setup();
    loadFromFile(pathFile);
}
//-----------------------------------------------------------------------------
Comic2::~Comic2()
{
	/*delete _7z;
	delete _7ze;
	delete bm;*/ //TODO safe delete
}
//-----------------------------------------------------------------------------
void Comic2::setup()
{
    connect(this,SIGNAL(pageChanged(int)),this,SLOT(checkIsBookmark(int)));
	connect(this,SIGNAL(imageLoaded(int)),this,SLOT(updateBookmarkImage(int)));
	connect(this,SIGNAL(imageLoaded(int)),this,SLOT(setPageLoaded(int)));
}
//-----------------------------------------------------------------------------
bool Comic2::load(const QString & path)
{
	QFileInfo fi(path);
	if(fi.exists())
	{
		if(fi.isFile())
		{
			loadFromFile(path);
		}
		else
		{
		}
		return true;
	}
	else
	{
		emit errorOpening();
		return false;
	}
}
//-----------------------------------------------------------------------------
void Comic2::loadFromFile(const QString & pathFile)
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
//-----------------------------------------------------------------------------
void Comic2::loadSizes()
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
		//QMessageBox::critical(NULL,tr("File error"),tr("File not found or not images in file"));
		emit errorOpening();
		return;
	}
	_pages.resize(_sizes.size());
	_loadedPages = QVector<bool>(_sizes.size(),false);

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
void Comic2::loadImages()
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
void Comic2::openingError(QProcess::ProcessError error)
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
int Comic2::nextPage()
{
	if(_index<_pages.size()-1)
		_index++;

	emit pageChanged(_index);

	return _index;
}
//-----------------------------------------------------------------------------
int Comic2::previousPage()
{
	if(_index>0)
		_index--;

	emit pageChanged(_index);

	return _index;
}
//-----------------------------------------------------------------------------
void Comic2::setIndex(unsigned int index)
{
	if(static_cast<int>(index)<_pages.size()-1)
		_index = index;
	else
		_index = _pages.size()-1;

	emit pageChanged(_index);
}
//-----------------------------------------------------------------------------
QPixmap * Comic2::currentPage()
{
	QPixmap * p = new QPixmap();
	p->loadFromData(_pages[_index]);
	return p;
}
//-----------------------------------------------------------------------------
QPixmap * Comic2::operator[](unsigned int index)
{
	QPixmap * p = new QPixmap();
	p->loadFromData(_pages[index]);
	return p;
}
//-----------------------------------------------------------------------------
bool Comic2::loaded()
{
	return _loaded;
}
//-----------------------------------------------------------------------------
void Comic2::loadFinished()
{
	emit imagesLoaded();
}
//-----------------------------------------------------------------------------
void Comic2::setBookmark()
{

}
//-----------------------------------------------------------------------------
void Comic2::removeBookmark()
{
}
//-----------------------------------------------------------------------------
void Comic2::saveBookmarks()
{

}
//-----------------------------------------------------------------------------
void Comic2::checkIsBookmark(int index)
{
}
//-----------------------------------------------------------------------------
void Comic2::updateBookmarkImage(int index)
{

}
//-----------------------------------------------------------------------------
void Comic2::setPageLoaded(int page)
{
	_loadedPages[page] = true;
}
//-----------------------------------------------------------------------------
QByteArray Comic2::getRawPage(int page)
{
	if(page < 0 || page >= _pages.size())
		return QByteArray();
	return _pages[page];
}
//-----------------------------------------------------------------------------
bool Comic2::pageIsLoaded(int page)
{
	if(page < 0 || page >= _pages.size())
		return false;
	return _loadedPages[page];
}