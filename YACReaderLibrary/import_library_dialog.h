#ifndef IMPORT_LIBRARY_DIALOG_H
#define IMPORT_LIBRARY_DIALOG_H
#include "yacreader_libraries.h"

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QThread>
#include <QProgressBar>

	class ImportLibraryDialog : public QDialog
	{
	Q_OBJECT
	public:
		ImportLibraryDialog(QWidget * parent = 0);
	private:
		QLabel * nameLabel;
		QLabel * textLabel;
		QLabel * destLabel;
		QLineEdit * path;
		QLineEdit * destPath;
		QLineEdit * nameEdit;
		QPushButton * find;
		QPushButton * findDest;
		QPushButton * accept;
		QPushButton * cancel;
		QProgressBar *progressBar;
		void setupUI();
		int progressCount;
		void closeEvent ( QCloseEvent * e );
		YACReaderLibraries  libraries;
	public slots:
		void add();
		void findPath();
		void findDestination();
		void close();
		void nameEntered();
		void open(const YACReaderLibraries & libs);

	signals:
		void unpackCLC(QString clc,QString targetFolder, QString name);
		void libraryExists(const QString & name);
	};

#endif
