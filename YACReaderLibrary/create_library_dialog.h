#ifndef __CREATE_LIBRARY_DIALOG_H
#define __CREATE_LIBRARY_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QThread>

	class CreateLibraryDialog : public QDialog
	{
	Q_OBJECT
	public:
		CreateLibraryDialog(QWidget * parent = 0);
	private:
		QLabel * nameLabel;
		QLabel * textLabel;
		QLabel * processLabel;
		QLabel * currentFileLabel;
		QLineEdit * path;
		QLineEdit * nameEdit;
		QPushButton * find;
		QPushButton * accept;
		QPushButton * cancel;
		void setupUI();
	public slots:
		void create();
		void findPath();
		void showCurrentFile(QString file);
		void close();
	signals:
		void createLibrary(QString source, QString target, QString name);
		void cancelCreate();
	};

	class UpdateLibraryDialog : public QDialog
	{
		Q_OBJECT
	public:
		UpdateLibraryDialog(QWidget * parent = 0);
	private:
		QLabel * message;
		QLabel * currentFileLabel;
		QPushButton * cancel;
		public slots:
			void showCurrentFile(QString file);
			void close();
		signals:
			void cancelUpdate();
	};

#endif
