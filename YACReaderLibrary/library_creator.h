#ifndef __LIBRARY_CREATOR_H
#define __LIBRARY_CREATOR_H

#include <QObject>
#include <QString>
#include <QDir>
#include <QFile>
#include <QByteArray>
#include <QRegExp>
#include <QProcess>
#include <QtCore>
#include <QtGui>
#include <QMutex>
#include <QThread>
#include <QSqlDatabase>


class LibraryItem
{
public:
	virtual bool isDir() = 0;
	virtual void removeFromDB(QSqlDatabase & db) = 0;
	QString name;
	QString path;
	qint64 parentId;
	qint64 id;
};

class Folder : public LibraryItem
{
public:
	bool knownParent;
	bool knownId;
	
	Folder():knownParent(false), knownId(false){};
	Folder(qint64 sid, qint64 pid,QString fn, QString fp):knownParent(true), knownId(true){id = sid; parentId = pid;name = fn; path = fp;};
	Folder(QString fn, QString fp):knownParent(false), knownId(false){name = fn; path = fp;};
	void setId(qint64 sid){id = sid;knownId = true;};
	void setFather(qint64 pid){parentId = pid;knownParent = true;};
	static QList<LibraryItem *> getFoldersFromParent(qint64 parentId, QSqlDatabase & db);
	qint64 insert(QSqlDatabase & db);
	bool isDir(){return true;};
	void removeFromDB(QSqlDatabase & db);
};

class Comic : public LibraryItem
{
public:
	qint64 comicInfoId;
	QString hash;

	Comic(){};
	Comic(qint64 cparentId, qint64 ccomicInfoId, QString cname, QString cpath, QString chash)
		:comicInfoId(ccomicInfoId),hash(chash){parentId = cparentId;name = cname; path = cpath;};
	//Comic(QString fn, QString fp):name(fn),path(fp),knownParent(false), knownId(false){};
	qint64 insert(QSqlDatabase & db);
	static QList<LibraryItem *> getComicsFromParent(qint64 parentId, QSqlDatabase & db);
	bool isDir(){return false;};
	void removeFromDB(QSqlDatabase & db);
};



	class LibraryCreator : public QThread
	{
		Q_OBJECT
	public:
		LibraryCreator();
		void createLibrary(const QString & source, const QString & target);
                void updateLibrary(const QString & source, const QString & target);
                void stop();
	private:
		enum Mode {CREATOR,UPDATER};
		//atributos "globales" durante el proceso de creación y actualización
		enum Mode _mode;
		QString _source;
		QString _target;
		QStringList _nameFilter;
		QSqlDatabase _database;
		QList<Folder> _currentPathFolders; //lista de folders en el orden en el que están siendo explorados, el último es el folder actual
		//recursive method
		void create(QDir currentDirectory);
		void update(QDir currentDirectory);
        void run();
		bool createTables();
		qint64 insertFolders();//devuelve el id del último folder añadido (último en la ruta)
		void insertComic(const QString & relativePath,const QFileInfo & fileInfo);
		//qint64 insertFolder(qint64 parentId,const Folder & folder);
		//qint64 insertComic(const Comic & comic);
        bool stopRunning;
	signals:
		void finished();
        void coverExtracted(QString);
        void folderUpdated(QString);
	};

	class ThumbnailCreator : public QObject
	{
		Q_OBJECT

	public:
		ThumbnailCreator(QString fileSource, QString target);
	private:
		QProcess * _7z;
		QString _fileSource;
		QString _target;
		QString _currentName;
		int _numPages;
		QPixmap _cover;

	public slots:
		void create();		
		int getNumPages(){return _numPages;};
		QPixmap getCover(){return _cover;};
	signals:
		void openingError(QProcess::ProcessError error);

	};

#endif
