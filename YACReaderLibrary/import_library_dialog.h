#ifndef IMPORT_LIBRARY_DIALOG_H
#define IMPORT_LIBRARY_DIALOG_H


#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QThread>
#include <QTimer>

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
		QLabel * progress;
		void setupUI();
		QTimer t;
		int progressCount;
		void closeEvent ( QCloseEvent * e );
	public slots:
		void add();
		void findPath();
		void findDestination();
		void close();
		void updateProgress();
		void nameEntered();

	signals:
		void unpackCLC(QString clc,QString targetFolder, QString name);
	};

#endif
