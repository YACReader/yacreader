#include "comic.h"

#include <QPixmap>
#include <QRegExp>
#include <QString>
#include <algorithm>
#include <QDir>
#include <QFileInfoList>
#include "bookmarks.h" //TODO desacoplar la dependencia con bookmarks
#include "qnaturalsorting.h"
#include "compressed_archive.h"
#include "comic_db.h"

//-----------------------------------------------------------------------------
Comic::Comic()
:_pages(),_index(0),_path(),_loaded(false),bm(new Bookmarks()),_loadedPages(),_isPDF(false)
{
    setup();
}
//-----------------------------------------------------------------------------
Comic::Comic(const QString & pathFile, int atPage )
:_pages(),_index(0),_path(pathFile),_loaded(false),bm(new Bookmarks()),_loadedPages(),_isPDF(false),_firstPage(atPage)
{
    setup();
}
//-----------------------------------------------------------------------------
Comic::~Comic()
{

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
	{
		_index++;

		emit pageChanged(_index);
	}
	else
		emit isLast();
	return _index;
}
//---------------------------------------------------------------------------
int Comic::previousPage()
{
	if(_index>0)
	{
		_index--;

		emit pageChanged(_index);
	}
	else
		emit isCover();

	return _index;
}
//-----------------------------------------------------------------------------
void Comic::setIndex(unsigned int index)
{
	int previousIndex = _index;
	if(static_cast<int>(index)<_pages.size()-1)
		_index = index;
	else
		_index = _pages.size()-1;

	if(previousIndex != _index)
		emit pageChanged(_index);
}
//-----------------------------------------------------------------------------
/*QPixmap * Comic::currentPage()
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
}*/
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
	QImage p;
	p.loadFromData(_pages[_index]);
    bm->setBookmark(_index,p);
	//emit bookmarksLoaded(*bm);
	emit bookmarksUpdated();
}
//-----------------------------------------------------------------------------
void Comic::removeBookmark()
{
	bm->removeBookmark(_index);
	//emit bookmarksLoaded(*bm);
	emit bookmarksUpdated();
}
//-----------------------------------------------------------------------------
void Comic::saveBookmarks()
{
	QImage p;
	p.loadFromData(_pages[_index]);
	bm->setLastPage(_index,p);
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
		QImage p;
		p.loadFromData(_pages[index]);
		bm->setBookmark(index,p);
		emit bookmarksUpdated();
		//emit bookmarksLoaded(*bm);
		
	}
	if(bm->getLastPage() == index)
	{
		QImage p;
		p.loadFromData(_pages[index]);
		bm->setLastPage(index,p);
		emit bookmarksUpdated();
		//emit bookmarksLoaded(*bm);
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

FileComic::FileComic(const QString & path, int atPage )
	:Comic(path,atPage)
{
	load(path,atPage);
}

FileComic::~FileComic()
{

}

bool FileComic::load(const QString & path, int atPage)
{
	QFileInfo fi(path);

	if(fi.exists())
	{
		if(atPage == -1)
		{
			bm->newComic(path);
			emit bookmarksUpdated();
		}
		_firstPage = atPage;
		//emit bookmarksLoaded(*bm);

		_path = QDir::cleanPath(path);
		//load files size

		return true;
	}
	else
	{
		//QMessageBox::critical(NULL,tr("Not found"),tr("Comic not found")+" : " + path);
		emit errorOpening();
		return false;
	}
}

bool FileComic::load(const QString & path, const ComicDB & comic)
{
	QFileInfo fi(path);

	if(fi.exists())
	{
		QList<int> bookmarkIndexes;
		bookmarkIndexes << comic.info.bookmark1 << comic.info.bookmark2 << comic.info.bookmark3;
		if(bm->load(bookmarkIndexes,comic.info.currentPage-1))
			emit bookmarksUpdated();
		_firstPage = comic.info.currentPage-1;
		_path = QDir::cleanPath(path);
		return true;
	}
	else
	{
		//QMessageBox::critical(NULL,tr("Not found"),tr("Comic not found")+" : " + path);
		emit errorOpening();
		return false;
	}
}

QList<QString> FileComic::filter(const QList<QString> & src)
{
	QList<QString> extensions;
	extensions EXTENSIONS_LITERAL;
	QList<QString> filtered;
	bool fileAccepted = false;

	foreach(QString fileName,src)
	{
		fileAccepted = false;
		if(!fileName.contains("__MACOSX"))
		{
			foreach(QString extension,extensions)
			{
				if(fileName.endsWith(extension,Qt::CaseInsensitive))
				{
					fileAccepted = true;
					break;
				}
			}
		}
		if(fileAccepted)
			filtered.append(fileName);
	}

	return filtered;
}

//DELEGATE methods
void FileComic::fileExtracted(int index, const QByteArray & rawData)
{
	/*QFile f("c:/temp/out2.txt");
	f.open(QIODevice::Append);
	QTextStream out(&f);*/
	int sortedIndex = _fileNames.indexOf(_order.at(index));
	//out << sortedIndex << " , ";
	//f.close();
	if(sortedIndex == -1)
		return;
	_pages[sortedIndex] = rawData;
	emit imageLoaded(sortedIndex);
	emit imageLoaded(sortedIndex,_pages[sortedIndex]);
}

void FileComic::crcError(int index)
{
	emit errorOpening(tr("CRC error: some of the pages will not be displayed correctly"));
}

//TODO: comprobar que si se produce uno de estos errores, la carga del cómic es irrecuperable
void FileComic::unknownError(int index)
{
	//emit errorOpening(tr("Unknown error opening the file"));
	//emit errorOpening();
}

//--------------------------------------

QList<QVector<quint32> > FileComic::getSections(int & sectionIndex)
{
	QVector<quint32> sortedIndexes;
	foreach(QString name, _fileNames)
	{
		sortedIndexes.append(_order.indexOf(name));
	}
	QList<QVector <quint32> > sections;
	quint32 previous = 0;
	sectionIndex = -1;
	int sectionCount = 0;
	QVector <quint32> section;
	int idx = 0;
	int realIdx;
	foreach(quint32 i, sortedIndexes)
	{
		
		if(_firstPage == idx)
		{
			sectionIndex = sectionCount;
			realIdx  = i;
		}
		if(previous <= i)
		{
			//out << "idx : " << i << endl;
			section.append(i);
			previous = i;
		}
		else
		{
			if(sectionIndex == sectionCount) //found
			{
				if(section.indexOf(realIdx)!=0)
				{
					QVector <quint32> section1;
					QVector <quint32> section2;
					foreach(quint32 si,section)
					{
						if(si<realIdx)
							section1.append(si);
						else
							section2.append(si);
					}
					sectionIndex++;
					sections.append(section1);
					sections.append(section2);
					//out << "SPLIT" << endl;

				}
				else
				{
					sections.append(section);
				}
			}
			else
				sections.append(section);

			section = QVector <quint32> ();
			//out << "---------------" << endl;
			section.append(i);
			//out << "idx : " << i << endl;
			previous = i;
			sectionCount++;
		}

		idx++;
	}
				if(sectionIndex == sectionCount) //found
			{
				if(section.indexOf(realIdx)!=0)
				{
					QVector <quint32> section1;
					QVector <quint32> section2;
					foreach(quint32 si,section)
					{
						if(si<realIdx)
							section1.append(si);
						else
							section2.append(si);
					}
					sectionIndex++;
					sections.append(section1);
					sections.append(section2);
					//out << "SPLIT" << endl;

				}
				else
				{
					sections.append(section);
				}
			}
			else
				sections.append(section);

	//out << "se han encontrado : " << sections.count() << " sectionIndex : " << sectionIndex << endl;
	return sections;
}

void FileComic::process()
{
	QTime myTimer;
	myTimer.start();
	// do something..

	CompressedArchive archive(_path);
	//se filtran para obtener sólo los formatos soportados
	_order = archive.getFileNames();
	_fileNames = filter(_order);

	if(_fileNames.size()==0)
	{
		//QMessageBox::critical(NULL,tr("File error"),tr("File not found or not images in file"));
		emit errorOpening();
		return;
	}

	//TODO, cambiar por listas
	//_order = _fileNames;

	_pages.resize(_fileNames.size());
	_loadedPages = QVector<bool>(_fileNames.size(),false);

	emit pageChanged(0); // this indicates new comic, index=0
	emit numPages(_pages.size());
	_loaded = true;

	_cfi=0;
	qSort(_fileNames.begin(),_fileNames.end(), naturalSortLessThanCI);
	int index = 0;
	int sortedIndex = 0;

	if(_firstPage == -1)
		_firstPage = bm->getLastPage();
	_index = _firstPage;
	emit(openAt(_index));

	int sectionIndex;
	QList<QVector <quint32> > sections = getSections(sectionIndex);

	for(int i = sectionIndex; i<sections.count() ; i++)
		archive.getAllData(sections.at(i),this);
	for(int i = 0; i<sectionIndex; i++)
		archive.getAllData(sections.at(i),this);
	//archive.getAllData(QVector<quint32>(),this);
	/*
	foreach(QString name,_fileNames)
	{
		index = _order.indexOf(name);
		sortedIndex = _fileNames.indexOf(name);
		_pages[sortedIndex] = allData.at(index);
		emit imageLoaded(sortedIndex);
		emit imageLoaded(sortedIndex,_pages[sortedIndex]);
	}*/

	emit imagesLoaded();
	moveToThread(QApplication::instance()->thread());
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FolderComic::FolderComic()
	:Comic()
{

}

FolderComic::FolderComic(const QString & path, int atPage )
	:Comic(path, atPage )
{
	load(path, atPage );
}

FolderComic::~FolderComic()
{

}

bool FolderComic::load(const QString & path, int atPage )
{
	_path = path;
	if(atPage == -1)
	{
		bm->newComic(_path);
		emit bookmarksUpdated();
	}
	_firstPage = atPage;
	//emit bookmarksLoaded(*bm);
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
		if(_firstPage == -1)
			_firstPage = bm->getLastPage();

		_index = _firstPage;

		emit(openAt(_index));

		emit pageChanged(0); // this indicates new comic, index=0
		emit numPages(_pages.size());
		_loaded = true;

		int count=0;
		int i=_firstPage;
		while(count<nPages)
		{
			QFile f(list.at(i).absoluteFilePath());
			f.open(QIODevice::ReadOnly);
			_pages[i]=f.readAll();
			emit imageLoaded(i);
			emit imageLoaded(i,_pages[i]);
			i++;
			if(i==nPages)
				i=0;
			count++;
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

PDFComic::PDFComic(const QString & path, int atPage)
	:Comic(path,atPage)
{
	load(path,atPage);
}

PDFComic::~PDFComic()
{

}

bool PDFComic::load(const QString & path, int atPage)
{	
	QFileInfo fi(path);

	if(fi.exists())
	{
		_path = path;
		if(atPage == -1)
		{
			bm->newComic(_path);
			emit bookmarksUpdated();
		}
		_firstPage = atPage;
		//emit bookmarksLoaded(*bm);
		return true;
	}
	else
	{
		emit errorOpening();
		return false;
	}
}

bool PDFComic::load(const QString & path, const ComicDB & comic)
{
	QFileInfo fi(path);

	if(fi.exists())
	{
		QList<int> bookmarkIndexes;
		bookmarkIndexes << comic.info.bookmark1 << comic.info.bookmark2 << comic.info.bookmark3;
		if(bm->load(bookmarkIndexes,comic.info.currentPage-1))
			emit bookmarksUpdated();
		_firstPage = comic.info.currentPage-1;
		_path = QDir::cleanPath(path);
		return true;
	}
	else
	{
		//QMessageBox::critical(NULL,tr("Not found"),tr("Comic not found")+" : " + path);
		emit errorOpening();
		return false;
	}
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

	if(_firstPage == -1)
		_firstPage = bm->getLastPage();
	_index = _firstPage;
	emit(openAt(_index));

	for(int i=_index;i<nPages;i++)
		renderPage(i);
	for(int i=0;i<_index;i++)
		renderPage(i);

	delete pdfComic;
	emit imagesLoaded();
	moveToThread(QApplication::instance()->thread());
}

void PDFComic::renderPage(int page)
{
	Poppler::Page* pdfpage = pdfComic->page(page);
	if (pdfpage)
	{
		QImage img = pdfpage->renderToImage(150,150);
		delete pdfpage;
		QByteArray ba;
		QBuffer buf(&ba);
		img.save(&buf, "jpg");
		_pages[page] = ba;
		emit imageLoaded(page);
		emit imageLoaded(page,_pages[page]);
	}
}

Comic * FactoryComic::newComic(const QString & path)
{

	QFileInfo fi(path);
	if(fi.exists())
	{
		if(fi.isFile())
		{
			if(fi.suffix().compare("pdf",Qt::CaseInsensitive) == 0)
				return new PDFComic();
			else
				return new FileComic();
		}
		else
		{
			if(fi.isDir())
				return new FolderComic();
			else
				return NULL;
		}
	}
	else
		return NULL;

}