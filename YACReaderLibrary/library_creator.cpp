#include "library_creator.h"
#include "custom_widgets.h"

#include <QMutex>
#include <QDebug>

//QMutex mutex;



/*int numThreads = 0;
QWaitCondition waitCondition;
QMutex mutex;
*/
LibraryCreator::LibraryCreator()
{
	_nameFilter << "*.cbr" << "*.cbz" << "*.rar" << "*.zip" << "*.tar";
}

void LibraryCreator::createLibrary(const QString &source, const QString &target)
{
	_source = source;
	_target = target;
	if(!QDir(target).exists())
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

void LibraryCreator::run()
{
	stopRunning = false;
	if(_mode == CREATOR)
		create(QDir(_source));
	else
		update(QDir(_source),QDir(_target));
	emit(finished());
}

void LibraryCreator::stop()
{
	stopRunning = true;
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
		if(fileInfo.isDir())
		{
			create(QDir(fileInfo.absoluteFilePath()));
		}
		else
		{
			dir.mkpath(_target+(QDir::cleanPath(fileInfo.absolutePath()).remove(_source)));
			emit(coverExtracted(QDir::cleanPath(fileInfo.absoluteFilePath()).remove(_source)));	

			ThumbnailCreator tc(QDir::cleanPath(fileInfo.absoluteFilePath()),_target+(QDir::cleanPath(fileInfo.absoluteFilePath()).remove(_source))+".jpg");
			tc.create();
		}
	}
}

void LibraryCreator::update(QDir dirS,QDir dirD)
{
	dirS.setNameFilters(_nameFilter);
	dirS.setFilter(QDir::AllDirs|QDir::Files|QDir::NoDotAndDotDot);
	dirS.setSorting(QDir::Name|QDir::IgnoreCase|QDir::LocaleAware|QDir::DirsFirst);
	QFileInfoList listS = dirS.entryInfoList();

	dirD.setNameFilters(QStringList()<<"*.jpg");
	dirD.setFilter(QDir::AllDirs|QDir::Files|QDir::NoDotAndDotDot);
	dirD.setSorting(QDir::Name|QDir::IgnoreCase|QDir::LocaleAware|QDir::DirsFirst);
	QFileInfoList listD = dirD.entryInfoList();

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
				QFileInfo fileInfoD = listD.at(j);
				if(fileInfoD.isDir())
				{
					delTree(QDir(fileInfoD.absoluteFilePath()));
					dirD.rmdir(fileInfoD.absoluteFilePath());
				}
				else
					dirD.remove(fileInfoD.absoluteFilePath());
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
				if(fileInfoS.isDir())
					create(QDir(fileInfoS.absoluteFilePath()));
				else
				{
					dirD.mkpath(_target+(QDir::cleanPath(fileInfoS.absolutePath()).remove(_source)));
					emit(coverExtracted(QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(_source)));
					ThumbnailCreator tc(QDir::cleanPath(fileInfoS.absoluteFilePath()),_target+(QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(_source))+".jpg");
					tc.create();
				}
			}
			updated = true;
		}
		if(!updated)
		{
			QFileInfo fileInfoS = listS.at(i);
			QFileInfo fileInfoD = listD.at(j);
			QString nameS = QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(QDir::cleanPath(fileInfoS.absolutePath())); //remove source
			QString nameD = QDir::cleanPath(fileInfoD.absoluteFilePath()).remove(QDir::cleanPath(fileInfoD.absolutePath())); //remove target

			int comparation = QString::localeAwareCompare(nameS,nameD);
			if(fileInfoS.isDir()&&fileInfoD.isDir())
				if(comparation == 0)//same folder, update
				{
					update(QDir(fileInfoS.absoluteFilePath()),QDir(fileInfoD.absoluteFilePath()));
					i++;
					j++;
				}
				else
					if(comparation < 0) //nameS doesn't exist on Target folder...
					{
						if(nameS!="/.yacreaderlibrary")
							create(QDir(fileInfoS.absoluteFilePath()));
						i++;
					}
					else //nameD no longer avaliable on Source folder...
					{
						if(nameS!="/.yacreaderlibrary")
						{
							delTree(QDir(fileInfoD.absoluteFilePath()));
							dirD.rmdir(fileInfoD.absoluteFilePath());
							j++;
						}
						else
							i++; //skip library directory
					}
			else // one of them(or both) is a file
				if(fileInfoS.isDir()) //this folder doesn't exist on library
				{
					if(nameS!="/.yacreaderlibrary") //skip .yacreaderlibrary folder
						create(QDir(fileInfoS.absoluteFilePath()));
					i++;
				}
				else
					if(fileInfoD.isDir()) //delete this folder from library
					{
						delTree(QDir(fileInfoD.absoluteFilePath()));
						dirD.rmdir(fileInfoD.absoluteFilePath());
						j++;
					}
					else //both are files  //BUG on windows (no case sensitive)
					{
						nameD.remove(nameD.size()-4,4);
						int comparation = QString::localeAwareCompare(nameS,nameD);
						if(comparation < 0) //create new thumbnail
						{
							dirD.mkpath(_target+(QDir::cleanPath(fileInfoS.absolutePath()).remove(_source)));
							emit(coverExtracted(QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(_source)));
							ThumbnailCreator tc(QDir::cleanPath(fileInfoS.absoluteFilePath()),_target+(QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(_source))+".jpg");
							tc.create();
							i++;
						}
						else
						{
							if(comparation > 0) //delete thumbnail
							{
								dirD.remove(fileInfoD.absoluteFilePath());
								QString tick = fileInfoD.absoluteFilePath();
								dirD.remove(tick.remove(tick.size()-3,3));
								dirD.remove(tick+"r");
								j++;
							}
							else //same file
							{
								if(fileInfoS.isFile() && fileInfoD.isFile())
								{
									if(fileInfoS.lastModified()>fileInfoD.lastModified())
									{
										dirD.mkpath(_target+(QDir::cleanPath(fileInfoS.absolutePath()).remove(_source)));
										emit(coverExtracted(QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(_source)));
										ThumbnailCreator tc(QDir::cleanPath(fileInfoS.absoluteFilePath()),_target+(QDir::cleanPath(fileInfoS.absoluteFilePath()).remove(_source))+".jpg");
										tc.create();
									}
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
