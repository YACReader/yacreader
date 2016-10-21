#include "library_creator.h"
#include "custom_widgets.h"

#include <QMutex>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QCoreApplication>
#include <QLibrary>

#include "data_base_management.h"
#include "qnaturalsorting.h"
#include "db_helper.h"

#include "compressed_archive.h"
#include "comic.h"
#include "pdf_comic.h"
#include "yacreader_global.h"

#include "QsLog.h"

#include <algorithm>
using namespace std;

//--------------------------------------------------------------------------------
LibraryCreator::LibraryCreator()
    :creation(false), partialUpdate(false)
{
    _nameFilter << Comic::comicExtensions;
}

void LibraryCreator::createLibrary(const QString &source, const QString &target)
{
	creation = true;
	processLibrary(source, target);
}

void LibraryCreator::updateLibrary(const QString &source, const QString &target)
{
    partialUpdate = false;
    processLibrary(source, target);
}

void LibraryCreator::updateFolder(const QString &source, const QString &target, const QString &sourceFolder, const QModelIndex & dest)
{
    partialUpdate = true;
    folderDestinationModelIndex = dest;

    _currentPathFolders.clear();
    _currentPathFolders.append(Folder(1,1,"root","/"));

    QString relativeFolderPath = sourceFolder;
    relativeFolderPath = relativeFolderPath.remove(QDir::cleanPath(source));

    if(relativeFolderPath.startsWith("/"))
    {
        relativeFolderPath = relativeFolderPath.remove(0,1);//remove firts '/'
    }

    QStringList folders;

    if(!relativeFolderPath.isEmpty()) //updating root
    {
        folders = relativeFolderPath.split('/');
    }

    QLOG_DEBUG() << "folders found in relative path : " << folders << "-" << relativeFolderPath;

    QSqlDatabase db = DataBaseManagement::loadDatabase(target);

    foreach (QString folderName, folders) 
    {
	if(folderName.isEmpty())
	{
            break;
	}
        qulonglong parentId = _currentPathFolders.last().id;
        _currentPathFolders.append(DBHelper::loadFolder(folderName, parentId, db));
        QLOG_DEBUG() << "Folder appended : " << _currentPathFolders.last().id << " " << _currentPathFolders.last().name << " with parent" << _currentPathFolders.last().parentId;
    }

    QSqlDatabase::removeDatabase(_database.connectionName());

    QLOG_DEBUG() << "Relative path : " << relativeFolderPath;

    _sourceFolder = sourceFolder;

    processLibrary(source, target);
}

void LibraryCreator::processLibrary(const QString & source, const QString & target)
{
	_source = source;
	_target = target;
	if(DataBaseManagement::checkValidDB(target+"/library.ydb")=="")
	{
		//se limpia el directorio ./yacreaderlibrary
		QDir d(target);
		d.removeRecursively();
		_mode = CREATOR;
	}
	else
	{		//
		_mode = UPDATER;
	}
}


//
void LibraryCreator::run()
{
	stopRunning = false;
#ifndef use_unarr
//check for 7z lib
#if defined Q_OS_UNIX && !defined Q_OS_MAC
    QLibrary *sevenzLib = new QLibrary(QString(LIBDIR)+"/p7zip/7z.so");
#else
    QLibrary *sevenzLib = new QLibrary(QCoreApplication::applicationDirPath()+"/utils/7z");
#endif
	
	if(!sevenzLib->load())
	{
		QLOG_ERROR() << "Loading 7z.dll : " + sevenzLib->errorString() << endl;
        QCoreApplication::exit(YACReader::SevenZNotFound);
		exit();
	}
	sevenzLib->deleteLater();
#endif
	if(_mode == CREATOR)
	{
		QLOG_INFO() << "Starting to create new library ( " << _source << "," << _target << ")";
		_currentPathFolders.clear();
		_currentPathFolders.append(Folder(1,1,"root","/"));
		//se crean los directorios .yacreaderlibrary y .yacreaderlibrary/covers
		QDir dir;
		dir.mkpath(_target+"/covers");

		//se crea la base de datos .yacreaderlibrary/library.ydb
		_database = DataBaseManagement::createDatabase("library",_target);//
		if(!_database.isOpen())
		{
			QLOG_ERROR() << "Unable to create data base" << _database.lastError().databaseText() + "-" + _database.lastError().driverText();
			emit failedCreatingDB(_database.lastError().databaseText() + "-" + _database.lastError().driverText());
			emit finished();
			creation = false;
			return; 
		}

		/*QSqlQuery pragma("PRAGMA foreign_keys = ON",_database);*/
		_database.transaction();
		//se crea la librería
		create(QDir(_source));
		_database.commit();
		_database.close();
		QSqlDatabase::removeDatabase(_database.connectionName());
		emit(created());
		QLOG_INFO() << "Create library END";
	}
	else
	{
		QLOG_INFO() << "Starting to update folder" << _sourceFolder << "in library ( " << _source << "," << _target << ")";
		if(!partialUpdate)
		{
			_currentPathFolders.clear();
			_currentPathFolders.append(Folder(1,1,"root","/"));
			QLOG_DEBUG() << "update whole library";
		}

		_database = DataBaseManagement::loadDatabase(_target);
		//_database.setDatabaseName(_target+"/library.ydb");
		if(!_database.open())
		{
			QLOG_ERROR() << "Unable to open data base" << _database.lastError().databaseText() + "-" + _database.lastError().driverText();
			emit failedOpeningDB(_database.lastError().databaseText() + "-" + _database.lastError().driverText());
			emit finished();
			creation = false;
			return;
		}
		QSqlQuery pragma("PRAGMA foreign_keys = ON",_database);
		_database.transaction();
		
		if(partialUpdate)
		{
			update(QDir(_sourceFolder));
		}
		else
		{
		update(QDir(_source));
		}
		_database.commit();
		_database.close();
		QSqlDatabase::removeDatabase(_target);
		//si estabamos en modo creación, se está añadiendo una librería que ya existía y se ha actualizado antes de añadirse.
		if(!partialUpdate)
		{
			if(!creation)
			{
				emit(updated());
			}
			else
			{
				emit(created());
			}
		}
		QLOG_INFO() << "Update library END";
	}
    //msleep(100);//TODO try to solve the problem with the udpate dialog (ya no se usa más...)
    if(partialUpdate)
    {
        emit updatedCurrentFolder(folderDestinationModelIndex);
        emit finished();
    }
    else //TODO check this part!!
        emit finished();
	creation = false;
}

void LibraryCreator::stop()
{
	_database.commit();
	stopRunning = true;
}

//retorna el id del ultimo de los folders
qulonglong LibraryCreator::insertFolders()
{
	QList<Folder>::iterator i;
	int currentId = 0;
	for (i = _currentPathFolders.begin(); i != _currentPathFolders.end(); ++i)
	{
		if(!(i->knownId))
		{
			i->setFather(currentId);
			currentId = DBHelper::insert(&(*i),_database);//insertFolder(currentId,*i);
			i->setId(currentId);
		}
		else
		{
			currentId = i->id;
		}
	}
    return currentId;
}

void LibraryCreator::create(QDir dir)
{
	dir.setNameFilters(_nameFilter);
	dir.setFilter(QDir::AllDirs|QDir::Files|QDir::NoDotAndDotDot);
	QFileInfoList list = dir.entryInfoList();
	for (int i = 0; i < list.size(); ++i) 
	{
		if(stopRunning)
			return;
		QFileInfo fileInfo = list.at(i);
		QString fileName = fileInfo.fileName();
#ifdef Q_OS_MAC
		QStringList src = _source.split("/");
		QString filePath = fileInfo.absoluteFilePath();
		QStringList fp = filePath.split("/");
		for(int i = 0; i< src.count();i++)
		{
			fp.removeFirst();
		}
		QString relativePath = "/" + fp.join("/");
#else
		QString relativePath = QDir::cleanPath(fileInfo.absoluteFilePath()).remove(_source);
#endif        
		if(fileInfo.isDir())
		{
            QLOG_TRACE() << "Parsing folder" << fileInfo.canonicalPath() ;
			//se añade al path actual el folder, aún no se sabe si habrá que añadirlo a la base de datos
			_currentPathFolders.append(Folder(fileInfo.fileName(),relativePath));
			create(QDir(fileInfo.absoluteFilePath()));
			//una vez importada la información del folder, se retira del path actual ya que no volverá a ser visitado
			_currentPathFolders.pop_back();
		}
		else
		{
            QLOG_TRACE() << "Parsing file" << fileInfo.filePath();
			insertComic(relativePath,fileInfo);
		}
	}
}

bool LibraryCreator::checkCover(const QString & hash)
{
	return QFile::exists(_target+"/covers/"+hash+".jpg");
}

void LibraryCreator::insertComic(const QString & relativePath,const QFileInfo & fileInfo)
{
	//Se calcula el hash del cómic

	QCryptographicHash crypto(QCryptographicHash::Sha1);
	QFile file(fileInfo.absoluteFilePath());
	file.open(QFile::ReadOnly);
	crypto.addData(file.read(524288));
	file.close();
	//hash Sha1 del primer 0.5MB + filesize
	QString hash = QString(crypto.result().toHex().constData()) + QString::number(fileInfo.size());
	ComicDB comic = DBHelper::loadComic(fileInfo.fileName(),relativePath,hash,_database);
	int numPages = 0;
	bool exists = checkCover(hash);
	if(! ( comic.hasCover() && exists))
	{
		ThumbnailCreator tc(QDir::cleanPath(fileInfo.absoluteFilePath()),_target+"/covers/"+hash+".jpg",comic.info.coverPage.toInt());
		tc.create();
		numPages = tc.getNumPages();
		if (numPages > 0)
		{
			emit(comicAdded(relativePath,_target+"/covers/"+hash+".jpg"));
		}
	}

	if (numPages > 0 || exists)
	{
		//en este punto sabemos que todos los folders que hay en _currentPath, deberían estar añadidos a la base de datos
		insertFolders();
		comic.info.numPages = numPages;
		comic.parentId = _currentPathFolders.last().id;
		DBHelper::insert(&comic,_database);
	}
}

void LibraryCreator::update(QDir dirS)
{
	//QLOG_TRACE() << "Updating" << dirS.absolutePath();
	//QLOG_TRACE() << "Getting info from dir" << dirS.absolutePath();
	dirS.setNameFilters(_nameFilter);
	dirS.setFilter(QDir::AllDirs|QDir::NoDotAndDotDot);
	dirS.setSorting(QDir::Name|QDir::IgnoreCase|QDir::LocaleAware);
	QFileInfoList listSFolders = dirS.entryInfoList();
	dirS.setFilter(QDir::Files|QDir::NoDotAndDotDot);
	dirS.setSorting(QDir::Name|QDir::IgnoreCase|QDir::LocaleAware);
	QFileInfoList listSFiles = dirS.entryInfoList();

	qSort(listSFolders.begin(),listSFolders.end(),naturalSortLessThanCIFileInfo);
	qSort(listSFiles.begin(),listSFiles.end(),naturalSortLessThanCIFileInfo);

	QFileInfoList listS;
	listS.append(listSFolders);
	listS.append(listSFiles);
	//QLOG_DEBUG() << "---------------------------------------------------------";
	//foreach(QFileInfo info,listS)
	//	QLOG_DEBUG() << info.fileName();

	//QLOG_TRACE() << "END Getting info from dir" << dirS.absolutePath();

	//QLOG_TRACE() << "Getting info from DB" << dirS.absolutePath();
	QList<LibraryItem *> folders = DBHelper::getFoldersFromParent(_currentPathFolders.last().id,_database);
	QList<LibraryItem *> comics = DBHelper::getComicsFromParent(_currentPathFolders.last().id,_database);
	//QLOG_TRACE() << "END Getting info from DB" << dirS.absolutePath();

	QList <LibraryItem *> listD;
	qSort(folders.begin(),folders.end(),naturalSortLessThanCILibraryItem);
	qSort(comics.begin(),comics.end(),naturalSortLessThanCILibraryItem);
	listD.append(folders);
	listD.append(comics);
	//QLOG_DEBUG() << "---------------------------------------------------------";
	//foreach(LibraryItem * info,listD)
	//	QLOG_DEBUG() << info->name;
	//QLOG_DEBUG() << "---------------------------------------------------------";
	int lenghtS = listS.size();
	int lenghtD = listD.size();
	//QLOG_DEBUG() << "S len" << lenghtS << "D len" << lenghtD;
	//QLOG_DEBUG() << "---------------------------------------------------------";

	bool updated;
	int i,j;
	for (i=0,j=0; (i < lenghtS)||(j < lenghtD);) 
	{
		if(stopRunning)
			return;
		updated = false;
		if(i>=lenghtS) //finished source files/dirs
		{
			//QLOG_WARN() << "finished source files/dirs" << dirS.absolutePath();
			//delete listD //from j
			for(;j<lenghtD;j++)
			{
				if(stopRunning)
					return;
				DBHelper::removeFromDB(listD.at(j),(_database));
			}
			updated = true;
		}
		if(j>=lenghtD) //finished library files/dirs
		{
			//QLOG_WARN() << "finished library files/dirs" << dirS.absolutePath();
			//create listS //from i
			for(;i<lenghtS;i++)
			{
				if(stopRunning)
					return;
				QFileInfo fileInfoS = listS.at(i);
				if(fileInfoS.isDir()) //create folder
				{
#ifdef Q_OS_MAC
					QStringList src = _source.split("/");
					QString filePath = fileInfoS.absoluteFilePath();
					QStringList fp = filePath.split("/");
					for(int i = 0; i< src.count();i++)
					{
						fp.removeFirst();
					}
					QString path = "/" + fp.join("/");
#else
					QString path = QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(_source);
#endif
					_currentPathFolders.append(Folder(fileInfoS.fileName(),path));  //folder actual no está en la BD
					create(QDir(fileInfoS.absoluteFilePath()));
					_currentPathFolders.pop_back();
				}
				else //create comic
				{
#ifdef Q_OS_MAC
					QStringList src = _source.split("/");
					QString filePath = fileInfoS.absoluteFilePath();
					QStringList fp = filePath.split("/");
					for(int i = 0; i< src.count();i++)
					{
						fp.removeFirst();
					}
					QString path = "/" + fp.join("/");
#else

					QString path = QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(_source);
#endif
					insertComic(path,fileInfoS);
				}
			}
			updated = true;
		}
		if(!updated)
		{
			QFileInfo fileInfoS = listS.at(i);
			LibraryItem * fileInfoD = listD.at(j);
			QString nameS = QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(QDir::cleanPath(fileInfoS.absolutePath())); //remove source
			QString nameD = "/"+fileInfoD->name;

			int comparation = QString::localeAwareCompare(nameS,nameD);
			if(fileInfoS.isDir()&&fileInfoD->isDir())
				if(comparation == 0)//same folder, update
				{
					_currentPathFolders.append(*static_cast<Folder *>(fileInfoD));//fileInfoD conoce su padre y su id
					update(QDir(fileInfoS.absoluteFilePath()));
					_currentPathFolders.pop_back();
					i++;
					j++;
				}
				else
					if(comparation < 0) //nameS doesn't exist on DB
					{

						if(nameS!="/.yacreaderlibrary")
						{
							//QLOG_WARN() << "dir source < dest" << nameS << nameD;
#ifdef Q_OS_MAC
							QStringList src = _source.split("/");
							QString filePath = fileInfoS.absoluteFilePath();
							QStringList fp = filePath.split("/");
							for(int i = 0; i< src.count();i++)
							{
								fp.removeFirst();
							}
							QString path = "/" + fp.join("/");
#else
							QString path = QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(_source);
#endif
							_currentPathFolders.append(Folder(fileInfoS.fileName(),path));
							create(QDir(fileInfoS.absoluteFilePath()));
							_currentPathFolders.pop_back();
						}
						i++;
					}
					else //nameD no longer available on Source folder...
					{
						if(nameS!="/.yacreaderlibrary")
						{
							//QLOG_WARN() << "dir source > dest" << nameS << nameD;
							DBHelper::removeFromDB(fileInfoD,_database);
							j++;
						}
						else
							i++; //skip library directory
					}
			else // one of them(or both) is a file
				if(fileInfoS.isDir()) //this folder doesn't exist on library
				{
					if(nameS!="/.yacreaderlibrary") //skip .yacreaderlibrary folder
					{
						//QLOG_WARN() << "one of them(or both) is a file" << nameS << nameD;
#ifdef Q_OS_MAC
						QStringList src = _source.split("/");
						QString filePath = fileInfoS.absoluteFilePath();
						QStringList fp = filePath.split("/");
						for(int i = 0; i< src.count();i++)
						{
							fp.removeFirst();
						}
						QString path = "/" + fp.join("/");
#else
						QString path = QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(_source);
#endif
						_currentPathFolders.append(Folder(fileInfoS.fileName(),path));
						create(QDir(fileInfoS.absoluteFilePath()));
						_currentPathFolders.pop_back();
					}
					i++;
				}
				else
					if(fileInfoD->isDir()) //delete this folder from library
					{
						DBHelper::removeFromDB(fileInfoD,_database);
						j++;
					}
					else //both are files  //BUG on windows (no case sensitive)
					{
						//nameD.remove(nameD.size()-4,4);
						int comparation = QString::localeAwareCompare(nameS,nameD);
						if(comparation < 0) //create new thumbnail
						{
#ifdef Q_OS_MAC
							QStringList src = _source.split("/");
							QString filePath = fileInfoS.absoluteFilePath();
							QStringList fp = filePath.split("/");
							for(int i = 0; i< src.count();i++)
							{
								fp.removeFirst();
							}
							QString path = "/" + fp.join("/");
#else
							QString path = QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(_source);
#endif
							insertComic(path,fileInfoS);
							i++;
						}
						else
						{
							if(comparation > 0) //delete thumbnail
							{
								DBHelper::removeFromDB(fileInfoD,_database);
								j++;
							}
							else //same file
							{
								if(fileInfoS.isFile() && !fileInfoD->isDir())
								{
									//TODO comprobar fechas + tamaño
									//if(fileInfoS.lastModified()>fileInfoD.lastModified())
									//{
									//	dirD.mkpath(_target+(QDir::cleanPath(fileInfoS.absolutePath()).remove(_source)));
									//	emit(coverExtracted(QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(_source)));
									//	ThumbnailCreator tc(QDir::cleanPath(fileInfoS.absoluteFilePath()),_target+(QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(_source))+".jpg");
									//	tc.create();
									//}
								}
								i++;j++;
							}
						}
					}
		}	
	}
}

bool ThumbnailCreator::crash = false;

ThumbnailCreator::ThumbnailCreator(QString fileSource, QString target, int coverPage)
:_fileSource(fileSource),_target(target),_numPages(0),_coverPage(coverPage)
{
}

void ThumbnailCreator::create()
{
	QFileInfo fi(_fileSource);
	if(!fi.exists()) //TODO: error file not found.
	{
		_cover.load(":/images/notCover.png");
		QLOG_WARN() << "Extracting cover: file not found " << _fileSource;
		return;
	}
#ifndef NO_PDF
	if(fi.suffix().compare("pdf",Qt::CaseInsensitive) == 0)
	{
#if defined Q_OS_MAC && defined USE_PDFKIT
		MacOSXPDFComic * pdfComic = new MacOSXPDFComic();
		if(!pdfComic->openComic(_fileSource))
		{
			delete pdfComic;
			//QImage p;
			//p.load(":/images/notCover.png");
			//p.save(_target);
			return;
		}
#elif defined USE_PDFIUM
		PdfiumComic * pdfComic =  new PdfiumComic();
		if(!pdfComic->openComic(_fileSource))
		{
			delete pdfComic;
			return;
		}
#else
		Poppler::Document * pdfComic = Poppler::Document::load(_fileSource);
#endif
	
		if (!pdfComic)
		{
			QLOG_WARN() << "Extracting cover: unable to open PDF file " << _fileSource;
			//delete pdfComic; //TODO check if the delete is needed
			pdfComic = 0;
			//QImage p;
			//p.load(":/images/notCover.png");
			//p.save(_target);
			return;
		}
#if !defined USE_PDFKIT && !defined USE_PDFIUM
		//poppler only, not mac
		if (pdfComic->isLocked())
		{
			QLOG_WARN() << "Extracting cover: unable to open PDF file " << _fileSource;
			delete pdfComic;
			return;
		}
#endif
		_numPages = pdfComic->numPages();
		if(_numPages >= _coverPage)
		{
#if defined Q_OS_MAC || defined USE_PDFIUM
			QImage p = pdfComic->getPage(_coverPage-1); //TODO check if the page is valid
		#ifdef USE_PDFKIT
			pdfComic->releaseLastPageData();
		#endif
#else
			QImage p = pdfComic->page(_coverPage-1)->renderToImage(72,72);
#endif //
			_cover = p;
			if(_target!="")
			{
				QImage scaled;
				if(p.width()>p.height()) //landscape??
				{
					scaled = p.scaledToWidth(640,Qt::SmoothTransformation);
				}
				else
				{
					scaled = p.scaledToWidth(480,Qt::SmoothTransformation);
				}
					scaled.save(_target,0,75);
			}
			else if(_target!="")
			{
				QLOG_WARN() << "Extracting cover: requested cover index greater than numPages " << _fileSource;
				//QImage p;
				//p.load(":/images/notCover.png");
				//p.save(_target);
			}
			delete pdfComic;
		}
		return;
	}
#endif //NO_PDF
	
	if(crash)
	{
		return;
	}

	CompressedArchive archive(_fileSource);
	if(!archive.toolsLoaded())
	{
		QLOG_WARN() << "Extracting cover: 7z lib not loaded";
		crash = true;
		return;
	}
	if(!archive.isValid())
	{
		QLOG_WARN() << "Extracting cover: file format not supported " << _fileSource;	
	}
	//se filtran para obtener sólo los formatos soportados
	QList<QString> order = archive.getFileNames();
	QList<QString> fileNames = FileComic::filter(order);
	_numPages = fileNames.size();
	if(_numPages == 0)
	{
		QLOG_WARN() << "Extracting cover: empty comic " << _fileSource;
		_cover.load(":/images/notCover.png");
		if(_target!="")
		{
			_cover.save(_target);
		}
	}
	else
	{
		if(_coverPage > _numPages)
		{
			_coverPage = 1;
		}
		qSort(fileNames.begin(),fileNames.end(), naturalSortLessThanCI);
		int index = order.indexOf(fileNames.at(_coverPage-1));

		if(_target=="")
		{
			if(!_cover.loadFromData(archive.getRawDataAtIndex(index)))
			{
				QLOG_WARN() << "Extracting cover: unable to load image from extracted cover " << _fileSource;
				_cover.load(":/images/notCover.png");
			}
		}
		else
		{
			QImage p;
			if(p.loadFromData(archive.getRawDataAtIndex(index)))
			{
				QImage scaled;
				if(p.width()>p.height()) //landscape??
				{
					scaled = p.scaledToWidth(640,Qt::SmoothTransformation);
				}
				else
				{
					scaled = p.scaledToWidth(480,Qt::SmoothTransformation);
				}
				scaled.save(_target,0,75);
			}
			else
			{
				QLOG_WARN() << "Extracting cover: unable to load image from extracted cover " << _fileSource;
		//p.load(":/images/notCover.png");
		//p.save(_target);
			}
		}
	}
}
