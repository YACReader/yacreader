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
		enum Mode _mode;
		QString _source;
		QString _target;
		QStringList _nameFilter;
		//recursive method
		void create(QDir currentDirectory);
		void update(QDir currentDirectory,QDir libraryCurrentDirectory);
        void run();
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
