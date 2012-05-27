#include "library_creator.h"
#include "custom_widgets.h"

#include <QMutex>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlRecord>
//QMutex mutex;



/*int numThreads = 0;
QWaitCondition waitCondition;
QMutex mutex;
*/



qint64 Folder::insert(QSqlDatabase & db)
{
	QSqlQuery query(db);
	query.prepare("INSERT INTO folder (parentId, name, path) "
                   "VALUES (:parentId, :name, :path)");
    query.bindValue(":parentId", parentId);
    query.bindValue(":name", name);
	query.bindValue(":path", path);
	query.exec();
	return query.lastInsertId().toLongLong();
}

QList<LibraryItem *> Folder::getFoldersFromParent(qint64 parentId, QSqlDatabase & db)
{
	QList<LibraryItem *> list;

	QSqlQuery selectQuery(db); //TODO check
	selectQuery.prepare("SELECT * FROM folder WHERE parentId = :parentId");
	selectQuery.bindValue(":parentId", parentId);
	selectQuery.exec();

	Folder * currentItem;
	while (selectQuery.next()) 
	{
		QList<QVariant> data;
		QSqlRecord record = selectQuery.record();
		for(int i=0;i<record.count();i++)
			data << record.value(i);
		//TODO sort by sort indicator and name
		currentItem = new Folder(record.value(0).toLongLong(),record.value(1).toLongLong(),record.value(2).toString(),record.value(2).toString());
		int lessThan = 0;
		if(list.isEmpty())
			list.append(currentItem);
		else
		{
			Folder * last = static_cast<Folder *>(list.back());
			QString nameLast = last->name; //TODO usar info name si est� disponible, sino el nombre del fichero.....
			QString nameCurrent = currentItem->name;
			QList<LibraryItem *>::iterator i;
			i = list.end();
			i--;
			while ((0 > (lessThan = nameCurrent.localeAwareCompare(nameLast))) && i != list.begin())
			{
				i--;
				nameLast = (*i)->name;
			}
			if(lessThan>0) //si se ha encontrado un elemento menor que current, se inserta justo despu�s
				list.insert(++i,currentItem);
			else
				list.insert(i,currentItem);

		}
	}

	return list;
}

void Folder::removeFromDB(QSqlDatabase & db)
{
	QSqlQuery query(db);
	query.prepare("DELETE FROM folder WHERE id = :id");
    query.bindValue(":id", id);
	query.exec();
}



qint64 Comic::insert(QSqlDatabase & db)
{
	//TODO comprobar si ya hay comic info con ese hash
	QSqlQuery comicInfoInsert(db);
	comicInfoInsert.prepare("INSERT INTO comic_info (hash) "
		"VALUES (:hash)");
	comicInfoInsert.bindValue(":hash", hash);
	 comicInfoInsert.exec();
	 qint64 comicInfoId =comicInfoInsert.lastInsertId().toLongLong();

	QSqlQuery query(db);
	query.prepare("INSERT INTO comic (parentId, comicInfoId, fileName, path) "
                   "VALUES (:parentId,:comicInfoId,:name, :path)");
    query.bindValue(":parentId", parentId);
	query.bindValue(":comicInfoId", comicInfoId);
    query.bindValue(":name", name);
	query.bindValue(":path", path);
	query.exec();
	return query.lastInsertId().toLongLong();
}

QList<LibraryItem *> Comic::getComicsFromParent(qint64 parentId, QSqlDatabase & db)
{
	QList<LibraryItem *> list;

	QSqlQuery selectQuery(db); //TODO check
	selectQuery.prepare("select c.id,c.parentId,c.fileName,c.path,ci.hash from comic c inner join comic_info ci on (c.comicInfoId = ci.id) where c.parentId = :parentId");
	selectQuery.bindValue(":parentId", parentId);
	selectQuery.exec();

	Comic * currentItem;
	while (selectQuery.next()) 
	{
		QList<QVariant> data;
		QSqlRecord record = selectQuery.record();
		for(int i=0;i<record.count();i++)
			data << record.value(i);
		//TODO sort by sort indicator and name
		currentItem = new Comic;
		currentItem->id = record.value(0).toLongLong();
		currentItem->parentId = record.value(1).toLongLong();
		currentItem->name = record.value(2).toString();
		currentItem->hash = record.value(3).toString();
		int lessThan = 0;
		if(list.isEmpty())
			list.append(currentItem);
		else
		{
			Comic * last = static_cast<Comic *>(list.back());
			QString nameLast = last->name; 
			QString nameCurrent = currentItem->name;
			QList<LibraryItem *>::iterator i;
			i = list.end();
			i--;
			while ((0 > (lessThan = nameCurrent.localeAwareCompare(nameLast))) && i != list.begin())  //se usa la misma ordenaci�n que en QDir
			{
				i--;
				nameLast = (*i)->name;
			}
			if(lessThan>0) //si se ha encontrado un elemento menor que current, se inserta justo despu�s
				list.insert(++i,currentItem);
			else
				list.insert(i,currentItem);

		}
	}

	return list;
}

void Comic::removeFromDB(QSqlDatabase & db)
{
	QSqlQuery query(db);
	query.prepare("DELETE FROM comic WHERE id = :id");
    query.bindValue(":id", id);
	query.exec();
}
//--------------------------------------------------------------------------------
LibraryCreator::LibraryCreator()
{
	_nameFilter << "*.cbr" << "*.cbz" << "*.rar" << "*.zip" << "*.tar";
}

void LibraryCreator::createLibrary(const QString &source, const QString &target)
{
	_source = source;
	_target = target;
	if(!QDir(target+"/library.ydb").exists())
		_mode = CREATOR;
	else
		_mode = UPDATER;
}

void LibraryCreator::updateLibrary(const QString &source, const QString &target)
{
	_source = source;
	_target = target;
	_mode = UPDATER;
}

bool LibraryCreator::createTables()
{
	bool success = true;

	//FOLDER (representa una carpeta en disco)
	QSqlQuery queryFolder(_database);
	queryFolder.prepare("CREATE TABLE folder (id INTEGER PRIMARY KEY, parentId INTEGER NOT NULL, name TEXT NOT NULL, path TEXT NOT NULL, FOREIGN KEY(parentId) REFERENCES folder(id) ON DELETE CASCADE)");
	success = success && queryFolder.exec();

		//COMIC INFO (representa la informaci�n de un c�mic, cada c�mic tendr� un id�ntificador �nico formado por un hash sha1'de los primeros 512kb' + su tama�o en bytes)
	QSqlQuery queryComicInfo(_database);
	queryComicInfo.prepare("CREATE TABLE comic_info (id INTEGER PRIMARY KEY, hash TEXT NOT NULL, name TEXT)");
	success = success && queryComicInfo.exec();

	//COMIC (representa un c�mic en disco, contiene el nombre de fichero)
	QSqlQuery queryComic(_database);
	queryComic.prepare("CREATE TABLE comic (id INTEGER PRIMARY KEY, parentId INTEGER NOT NULL, comicInfoId INTEGER NOT NULL,  fileName TEXT NOT NULL, path TEXT, FOREIGN KEY(parentId) REFERENCES folder(id) ON DELETE CASCADE, FOREIGN KEY(comicInfoId) REFERENCES comic_info(id))");
	success = success && queryComic.exec();

	return success;
}
//
void LibraryCreator::run()
{
	stopRunning = false;

	if(_mode == CREATOR)
	{
		QDir dir;
		dir.mkpath(_target+"/covers");
		_database = QSqlDatabase::addDatabase("QSQLITE");
		_database.setDatabaseName(_target+"/library.ydb");
		if(!_database.open())
			return; //TODO avisar del problema
		_database.transaction();
		_currentPathFolders.clear();
		_currentPathFolders.append(Folder(0,0,"root","/"));
		if(!createTables())
			return;
		create(QDir(_source));
		_database.commit();
		_database.close();
	}
	else
	{
		_currentPathFolders.clear();
		_currentPathFolders.append(Folder(0,0,"root","/"));
		_database = QSqlDatabase::addDatabase("QSQLITE");
		_database.setDatabaseName(_target+"/library.ydb");
		if(!_database.open())
			return; //TODO avisar del problema
		_database.transaction();
		update(QDir(_source));
		_database.commit();
		_database.close();
	}
	emit(finished());
}

void LibraryCreator::stop()
{
	_database.commit(); //TODO check
	stopRunning = true;
}

//retorna el id del ultimo de los folders
qint64 LibraryCreator::insertFolders()
{
	QList<Folder>::iterator i;
	int currentId = 0;
	for (i = _currentPathFolders.begin(); i != _currentPathFolders.end(); ++i)
	{
		if(!(i->knownId))
		{
			i->setFather(currentId);
			currentId = i->insert(_database);//insertFolder(currentId,*i);
			i->setId(currentId);
		}
		else
		{
			currentId = i->id;
		}
	}
	return 0;
}

/*qint64 LibraryCreator::insertFolder(qint64 parentId,const Folder & folder)
{
	QSqlQuery query(_database);
	query.prepare("INSERT INTO folder (parentId, name, path) "
                   "VALUES (:parentId, :name, :path)");
    query.bindValue(":parentId", parentId);
    query.bindValue(":name", folder.name);
	query.bindValue(":path", folder.path);
	query.exec();
	return query.lastInsertId().toLongLong();
}*/

/*qint64 LibraryCreator::insertComic(const Comic & comic)
{
	//TODO comprobar si ya hay comic info con ese hash
	QSqlQuery comicInfoInsert(_database);
	comicInfoInsert.prepare("INSERT INTO comic_info (hash) "
		"VALUES (:hash)");
	comicInfoInsert.bindValue(":hash", comic.hash);
	 comicInfoInsert.exec();
	 qint64 comicInfoId =comicInfoInsert.lastInsertId().toLongLong();

	QSqlQuery query(_database);
	query.prepare("INSERT INTO comic (parentId, comicInfoId, fileName, path) "
                   "VALUES (:parentId,:comicInfoId,:name, :path)");
    query.bindValue(":parentId", comic.parentId);
	query.bindValue(":comicInfoId", comicInfoId);
    query.bindValue(":name", comic.name);
	query.bindValue(":path", comic.path);
	query.exec();
	return query.lastInsertId().toLongLong();
}*/

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
		QString relativePath = QDir::cleanPath(fileInfo.absoluteFilePath()).remove(_source);
		if(fileInfo.isDir())
		{
			//se a�ade al path actual el folder, a�n no se sabe si habr� que a�adirlo a la base de datos
			_currentPathFolders.append(Folder(fileInfo.fileName(),relativePath));
			create(QDir(fileInfo.absoluteFilePath()));
			//una vez importada la informaci�n del folder, se retira del path actual ya que no volver� a ser visitado
			_currentPathFolders.pop_back();
		}
		else
		{
			insertComic(relativePath,fileInfo);
		}
	}
}

void LibraryCreator::insertComic(const QString & relativePath,const QFileInfo & fileInfo)
{
	//en este punto sabemos que todos los folders que hay en _currentPath, deber�an estar a�adidos a la base de datos
	insertFolders();
	emit(coverExtracted(relativePath));

	//Se calcula el hash del c�mic

	QCryptographicHash crypto(QCryptographicHash::Sha1);
	QFile file(fileInfo.absoluteFilePath());
	file.open(QFile::ReadOnly);
	crypto.addData(file.read(524288));
	file.close();
	//hash Sha1 del primer 0.5MB + filesize
	QString hash = QString(crypto.result().toHex().constData()) + QString::number(fileInfo.size());
	Comic(_currentPathFolders.last().id,0,fileInfo.fileName(),relativePath,hash).insert(_database);
	ThumbnailCreator tc(QDir::cleanPath(fileInfo.absoluteFilePath()),_target+"/covers/"+hash+".jpg");
	//ThumbnailCreator tc(QDir::cleanPath(fileInfo.absoluteFilePath()),_target+"/covers/"+fileInfo.fileName()+".jpg");
	tc.create();
}

void LibraryCreator::update(QDir dirS)
{
	dirS.setNameFilters(_nameFilter);
	dirS.setFilter(QDir::AllDirs|QDir::Files|QDir::NoDotAndDotDot);
	dirS.setSorting(QDir::Name|QDir::IgnoreCase|QDir::LocaleAware|QDir::DirsFirst); //TODO la ordenaci�n debe ser igual que en la base de datos
	//TODO obtener primero los directorios, despu�s los ficheros, ordenar por separado y concatenar
	QFileInfoList listS = dirS.entryInfoList();

	QList<LibraryItem *> folders = Folder::getFoldersFromParent(_currentPathFolders.last().id,_database);
	QList<LibraryItem *> comics = Comic::getComicsFromParent(_currentPathFolders.last().id,_database);

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
				listD.at(j)->removeFromDB(_database);
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
					_currentPathFolders.append(Folder(fileInfoS.fileName(),QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(_source)));  //folder actual no est� en la BD
					create(QDir(fileInfoS.absoluteFilePath()));
					_currentPathFolders.pop_back();
				}
				else //create comic
				{
					insertComic(QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(_source),fileInfoS);
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
							_currentPathFolders.append(Folder(fileInfoS.fileName(),QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(_source)));
							create(QDir(fileInfoS.absoluteFilePath()));
							_currentPathFolders.pop_back();
						}
						i++;
					}
					else //nameD no longer avaliable on Source folder...
					{
						if(nameS!="/.yacreaderlibrary")
						{
							fileInfoD->removeFromDB(_database);
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
						_currentPathFolders.append(Folder(fileInfoS.fileName(),QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(_source)));
						create(QDir(fileInfoS.absoluteFilePath()));
						_currentPathFolders.pop_back();
					}
					i++;
				}
				else
					if(fileInfoD->isDir()) //delete this folder from library
					{
						fileInfoD->removeFromDB(_database);
						j++;
					}
					else //both are files  //BUG on windows (no case sensitive)
					{
						//nameD.remove(nameD.size()-4,4);
						int comparation = QString::localeAwareCompare(nameS,nameD);
						if(comparation < 0) //create new thumbnail
						{
							insertComic(QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(_source),fileInfoS);
							i++;
						}
						else
						{
							if(comparation > 0) //delete thumbnail
							{
								fileInfoD->removeFromDB(_database);
								j++;
							}
							else //same file
							{
								if(fileInfoS.isFile() && !fileInfoD->isDir())
								{
									//TODO comprobar fechas + tama�o
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
ThumbnailCreator::ThumbnailCreator(QString fileSource, QString target="")
:_fileSource(fileSource),_target(target),_numPages(0)
{
}

void ThumbnailCreator::create()
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
	foreach(line,lines)
	{
		if(rx.indexIn(line)!=-1)
		{
			name = rx.cap(3).trimmed();
			if(0 > QString::localeAwareCompare(name,_currentName))
				_currentName = name;
			_numPages++;
		}	
	}
	delete _7z;
	attributes.clear();
	_currentName = QDir::fromNativeSeparators(_currentName).split('/').last(); //separator fixed. 
#ifdef Q_WS_WIN
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
			//TODO calculate aspect ratio
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
			//TODO save a default image.
		}
	}
	delete _7z;
}

/*void ThumbnailCreator::openingError(QProcess::ProcessError error)
{
	//TODO : move to the gui thread 
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
}*/
