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
:_pages(),_index(0),_path(),_loaded(false),bm(new Bookmarks()),_loadedPages(),_isPDF(false)
{
    setup();
}
//-----------------------------------------------------------------------------
Comic::Comic(const QString & pathFile)
:_pages(),_index(0),_path(pathFile),_loaded(false),bm(new Bookmarks()),_loadedPages(),_isPDF(false)
{
    setup();
}
//-----------------------------------------------------------------------------
Comic::~Comic()
{
	delete bm; //TODO safe delete
}
//-----------------------------------------------------------------------------
void Comic::setup()
{
    connect(this,SIGNAL(pageChanged(int)),this,SLOT(checkIsBookmark(int)));
	connect(this,SIGNAL(imageLoaded(int)),this,SLOT(updateBookmarkImage(int)));
	connect(this,SIGNAL(imageLoaded(int)),this,SLOT(setPageLoaded(int)));
}
//-----------------------------------------------------------------------------
int Comic::nextPage()
{
	if(_index<_pages.size()-1)
		_index++;

	emit pageChanged(_index);

	return _index;
}
//---------------------------------------------------------------------------
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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FileComic::FileComic()
	:Comic()
{

}

FileComic::FileComic(const QString & path)
	:Comic(path)
{
	load(path);
}

FileComic::~FileComic()
{
	//Comic::~Comic();
}

bool FileComic::load(const QString & path)
{
	QFileInfo fi(path);

	if(fi.exists())
	{
		bm->newComic(path);
		emit bookmarksLoaded(*bm);

		_path = QDir::cleanPath(path);
		//load files size
		_7z = new QProcess();
		QStringList attributes;
		attributes << "l" << "-ssc-" << "-r" << _path EXTENSIONS;
		connect(_7z,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(loadSizes(void)));
		connect(_7z,SIGNAL(error(QProcess::ProcessError)),this,SLOT(openingError(QProcess::ProcessError)));
		_7z->start(QCoreApplication::applicationDirPath()+"/utils/7z",attributes);

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
void FileComic::loadSizes()
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
void FileComic::loadImages()
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
void FileComic::openingError(QProcess::ProcessError error)
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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FolderComic::FolderComic()
	:Comic()
{

}

FolderComic::FolderComic(const QString & path)
	:Comic(path)
{
	load(path);
}

FolderComic::~FolderComic()
{

}

bool FolderComic::load(const QString & path)
{
	_path = path;
	return true;
}

void FolderComic::process()
{
	QDir d(_path);
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
		//TODO emitir este mensaje en otro sitio
		//QMessageBox::critical(NULL,QObject::tr("No images found"),QObject::tr("There are not images on the selected folder"));
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

	moveToThread(QApplication::instance()->thread());
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

PDFComic::PDFComic()
	:Comic()
{

}

PDFComic::PDFComic(const QString & path)
	:Comic(path)
{
	load(path);
}

PDFComic::~PDFComic()
{

}

bool PDFComic::load(const QString & path)
{
	_path = path;
	return true;
}

void PDFComic::process()
{
	pdfComic = Poppler::Document::load(_path);
	if (!pdfComic)
	{
		delete pdfComic;
		pdfComic = 0;
		//TODO emitir este mensaje en otro sitio
		//QMessageBox::critical(NULL,QObject::tr("Bad PDF File"),QObject::tr("Invalid PDF file"));
		emit errorOpening();
		return;
	}

	//pdfComic->setRenderHint(Poppler::Document::Antialiasing, true);
	pdfComic->setRenderHint(Poppler::Document::TextAntialiasing, true);
	int nPages = pdfComic->numPages();
	emit pageChanged(0); // this indicates new comic, index=0
	emit numPages(nPages);
	_loaded = true;
	//QMessageBox::critical(NULL,QString("%1").arg(nPages),tr("Invalid PDF file"));

	_pages.clear();
	_pages.resize(nPages);
	_loadedPages = QVector<bool>(nPages,false);
	for(int i=0;i<nPages;i++)
	{

		Poppler::Page* pdfpage = pdfComic->page(i);
		if (pdfpage)
		{
			QImage img = pdfpage->renderToImage(150,150); //TODO use defaults if not using X11 (e.g. MS Win)
			delete pdfpage;
			QByteArray ba;
			QBuffer buf(&ba);
			img.save(&buf, "jpg");
			_pages[i] = ba;
			emit imageLoaded(i);
			emit imageLoaded(i,_pages[i]);

		}

	}
	delete pdfComic;
	emit imagesLoaded();

	moveToThread(QApplication::instance()->thread());
}


Comic * FactoryComic::newComic(const QString & path)
{

	QFileInfo fi(path);
	if(fi.exists())
		if(fi.isFile())
			if(fi.suffix().compare("pdf",Qt::CaseInsensitive) == 0)
				return new PDFComic();
			else
				return new FileComic();
		else
			if(fi.isDir())
				return new FolderComic();
	else
		return NULL;

}