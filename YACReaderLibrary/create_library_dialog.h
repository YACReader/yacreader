#ifndef __CREATE_LIBRARY_DIALOG_H
#define __CREATE_LIBRARY_DIALOG_H

#include "yacreader_libraries.h"

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QThread>
#include <QProgressBar>

	class CreateLibraryDialog : public QDialog
	{
	Q_OBJECT
	public:
		CreateLibraryDialog(QWidget * parent = 0);
	private:
		QLabel * nameLabel;
		QLabel * textLabel;
		QLabel * message;
		QProgressBar *progressBar;
		QLineEdit * path;
		QLineEdit * nameEdit;
		QPushButton * find;
		QPushButton * accept;
		QPushButton * cancel;
		YACReaderLibraries  libraries;
		void setupUI();
	public slots:
		void create();
		void findPath();
		void close();
		void setDataAndStart(QString name, QString paht);
		void nameSetted(const QString & text);
		void pathSetted(const QString & text);
        void open(const YACReaderLibraries &libraries);
	signals:
		void createLibrary(QString source, QString target, QString name);
		void cancelCreate();
		void libraryExists(const QString & name);
	};

	class UpdateLibraryDialog : public QDialog
	{
		Q_OBJECT
	public:
		UpdateLibraryDialog(QWidget * parent = 0);
	private:
		QLabel * message;
		QLabel * currentFileLabel;
		QProgressBar *progressBar;
		QPushButton * cancel;
		public slots:
			void showCurrentFile(QString file);
			void close();
		signals:
			void cancelUpdate();
	};

#endif
