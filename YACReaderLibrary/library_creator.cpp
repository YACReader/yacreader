#include "library_creator.h"
#include "custom_widgets.h"

#include <QMutex>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlRecord>
#include "data_base_management.h"
#include "qnaturalsorting.h"
#include "db_helper.h"

#include <algorithm>
using namespace std;

#include "poppler-qt4.h"

//--------------------------------------------------------------------------------
LibraryCreator::LibraryCreator()
	:creation(false)
{
	_nameFilter << "*.cbr" << "*.cbz" << "*.rar" << "*.zip" << "*.tar" << "*.pdf" << "*.7z" << "*.cb7" << "*.arj" << "*.cbt";
}

void LibraryCreator::createLibrary(const QString &source, const QString &target)
{
	creation = true;
	processLibrary(source,target);
}

void LibraryCreator::updateLibrary(const QString &source, const QString &target)
{
	processLibrary(source,target);
}

void LibraryCreator::processLibrary(const QString & source, const QString & target)
{
	_source = source;
	_target = target;
	if(DataBaseManagement::checkValidDB(target+"/library.ydb")=="")
	{
		//se limpia el directorio ./yacreaderlibrary
		delTree(target);
		_mode = CREATOR;
	}
	else //
		_mode = UPDATER;
}


//
void LibraryCreator::run()
{
	stopRunning = false;

	if(_mode == CREATOR)
	{
		_currentPathFolders.clear();
		_currentPathFolders.append(Folder(1,1,"root","/"));
		//se crean los directorios .yacreaderlibrary y .yacreaderlibrary/covers
		QDir dir;
		dir.mkpath(_target+"/covers");

		//se crea la base de datos .yacreaderlibrary/library.ydb
		_database = DataBaseManagement::createDatabase("library",_target);//
		if(!_database.isOpen())
		{
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
	}
	else
	{
		_currentPathFolders.clear();
		_currentPathFolders.append(Folder(1,1,"root","/"));
		_database = DataBaseManagement::loadDatabase(_target);
		//_database.setDatabaseName(_target+"/library.ydb");
		if(!_database.open())
		{
			emit failedOpeningDB(_database.lastError().databaseText() + "-" + _database.lastError().driverText());
			emit finished();
			creation = false;
			return;
		}
		//QSqlQuery pragma("PRAGMA foreign_keys = ON",_database);
		_database.transaction();
		update(QDir(_source));
		_database.commit();
		_database.close();
		QSqlDatabase::removeDatabase(_target);
		//si estabamos en modo creación, se está añadiendo una librería que ya existía y se ha actualizado antes de añadirse.
		if(!creation)
			emit(updated());
		else
			emit(created());
	}
	msleep(100);//TODO try to solve the problem with the udpate dialog (ya no se usa más...)
	emit(finished());
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
	return 0;
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
			//se añade al path actual el folder, aún no se sabe si habrá que añadirlo a la base de datos
			_currentPathFolders.append(Folder(fileInfo.fileName(),relativePath));
			create(QDir(fileInfo.absoluteFilePath()));
			//una vez importada la información del folder, se retira del path actual ya que no volverá a ser visitado
			_currentPathFolders.pop_back();
		}
		else
		{
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
	//en este punto sabemos que todos los folders que hay en _currentPath, deberían estar añadidos a la base de datos
	insertFolders();
	emit(coverExtracted(relativePath));

	//Se calcula el hash del cómic

	QCryptographicHash crypto(QCryptographicHash::Sha1);
	QFile file(fileInfo.absoluteFilePath());
	file.open(QFile::ReadOnly);
	crypto.addData(file.read(524288));
	file.close();
	//hash Sha1 del primer 0.5MB + filesize
	QString hash = QString(crypto.result().toHex().constData()) + QString::number(fileInfo.size());
	ComicDB comic = DBHelper::loadComic(_currentPathFolders.last().id,fileInfo.fileName(),relativePath,hash,_database);
	int numPages;

	if(! ( comic.hasCover() && checkCover(hash)))
	{
		ThumbnailCreator tc(QDir::cleanPath(fileInfo.absoluteFilePath()),_target+"/covers/"+hash+".jpg",*comic.info.coverPage);
		//ThumbnailCreator tc(QDir::cleanPath(fileInfo.absoluteFilePath()),_target+"/covers/"+fileInfo.fileName()+".jpg");
		tc.create();
		numPages = tc.getNumPages();

		emit(comicAdded(relativePath,_target+"/covers/"+hash+".jpg"));
	}
	comic.info.setNumPages(numPages);
	DBHelper::insert(&comic,_database);
}

void LibraryCreator::update(QDir dirS)
{
	dirS.setNameFilters(_nameFilter);
	dirS.setFilter(QDir::AllDirs|QDir::Files|QDir::NoDotAndDotDot);
	dirS.setSorting(QDir::Name|QDir::IgnoreCase|QDir::LocaleAware|QDir::DirsFirst); 
	QFileInfoList listS = dirS.entryInfoList();

	QList<LibraryItem *> folders = DBHelper::getFoldersFromParent(_currentPathFolders.last().id,_database);
	QList<LibraryItem *> comics = DBHelper::getComicsFromParent(_currentPathFolders.last().id,_database);

	QList <LibraryItem *> listD;
	listD.append(folders);
	listD.append(comics);

	int lenghtS = listS.size();
	int lenghtD = listD.size();
	int maxLenght = qMax(lenghtS,lenghtD);

	bool updated;
	int i,j;
	for (i=0,j=0; (i < lenghtS)||(j < lenghtD);) 
	{
		if(stopRunning)
			return;
		updated = false;
		if(i>=lenghtS) //finished source files/dirs
		{
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
					else //nameD no longer avaliable on Source folder...
					{
						if(nameS!="/.yacreaderlibrary")
						{
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
ThumbnailCreator::ThumbnailCreator(QString fileSource, QString target="", int coverPage)
:_fileSource(fileSource),_target(target),_numPages(0),_coverPage(coverPage)
{
}

void ThumbnailCreator::create()
{
	QFileInfo fi(_fileSource);
	if(fi.suffix().compare("pdf",Qt::CaseInsensitive) == 0)
	{
		Poppler::Document * pdfComic = Poppler::Document::load(_fileSource);
		if (!pdfComic)
		{
			delete pdfComic;
			pdfComic = 0;
			QImage p;
			p.load(":/images/notCover.png");
			p.save(_target);
			return;
		}
		_numPages = pdfComic->numPages();
		if(_numPages >= _coverPage)
		{

			QImage p = pdfComic->page(_coverPage-1)->renderToImage(72,72); //TODO check if the the page is valid
			_cover = QPixmap::fromImage(p);
			QImage scaled;
			if(p.width()>p.height()) //landscape??
				scaled = p.scaledToWidth(640,Qt::SmoothTransformation);
			else
				scaled = p.scaledToWidth(480,Qt::SmoothTransformation);
			scaled.save(_target,0,75);
		}
		else
		{
			QImage p;
			p.load(":/images/notCover.png");
			p.save(_target);
		}
	}
	else
	{

		_7z = new QProcess();
		QStringList attributes;
		attributes << "l" << "-ssc-" << "-r" << _fileSource << "*.jpg" << "*.jpeg" << "*.png" << "*.gif" << "*.tiff" << "*.tif" << "*.bmp"; 
		//connect(_7z,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(loadFirstImage(void)));
		connect(_7z,SIGNAL(error(QProcess::ProcessError)),this,SIGNAL(openingError(QProcess::ProcessError)));
		_7z->start(QCoreApplication::applicationDirPath()+"/utils/7z",attributes);
		_7z->waitForFinished(60000);

		QRegExp rx("[0-9]{4}-[0-9]{2}-[0-9]{2}[ ]+[0-9]{2}:[0-9]{2}:[0-9]{2}[ ]+.{5}[ ]+([0-9]+)[ ]+([0-9]+)[ ]+(.+)");

		QString ba = QString::fromUtf8(_7z->readAllStandardOutput());
		QList<QString> lines = ba.split('\n');
		QString line;
		_currentName = "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ"; //TODO 
		QString name;
		if(_coverPage == 1)
		{
			foreach(line,lines)
			{
				if(rx.indexIn(line)!=-1)
				{
					name = rx.cap(3).trimmed();
					if(naturalSortLessThanCI(name,_currentName))
						_currentName = name;
					_numPages++;
				}	
			}
		}
		else
		{
			QList<QString> names;
			foreach(line,lines)
			{
				if(rx.indexIn(line)!=-1)
				{
					name = rx. cap(3).trimmed();
					names.append(name);
				}	
			}
			std::sort(names.begin(),names.end(),naturalSortLessThanCI);
			_currentName = names[_coverPage-1];
		}
		delete _7z;
		attributes.clear();
		_currentName = QDir::fromNativeSeparators(_currentName).split('/').last(); //separator fixed. 
#ifdef Q_OS_WIN32
		attributes << "e" << "-so" << "-r" << _fileSource << QString(_currentName.toLocal8Bit().constData()); //TODO platform dependency?? OEM 437
#else
		attributes << "e" << "-so" << "-r" << _fileSource << _currentName; //TODO platform dependency?? OEM 437
#endif
		_7z = new QProcess();
		connect(_7z,SIGNAL(error(QProcess::ProcessError)),this,SIGNAL(openingError(QProcess::ProcessError)));
		//connect(_7z,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(writeThumbnail(void)));
		_7z->start(QCoreApplication::applicationDirPath()+"/utils/7z",attributes);
		_7z->waitForFinished(60000);

		QByteArray image = _7z->readAllStandardOutput();
		QString error = _7z->readAllStandardError();
		QImage p;
		if(_target=="")
		{
			if(!_cover.loadFromData(image))
				_cover.load(":/images/notCover.png");
		}
		else
		{
			if(p.loadFromData(image))
			{
				QImage scaled;
				if(p.width()>p.height()) //landscape??
					scaled = p.scaledToWidth(640,Qt::SmoothTransformation);
				else
					scaled = p.scaledToWidth(480,Qt::SmoothTransformation);
				scaled.save(_target,0,75);
			}
			else
			{
				p.load(":/images/notCover.png");
				p.save(_target);
			}
		}
		delete _7z;
	}
}
