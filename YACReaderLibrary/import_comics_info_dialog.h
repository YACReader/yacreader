#ifndef IMPORT_COMICS_INFO_DIALOG_H
#define IMPORT_COMICS_INFO_DIALOG_H

#include <QDialog>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QThread>

class Importer : public QThread
{
public:
	QString source;
	QString dest;
private:
	void run();
};

class ImportComicsInfoDialog : public QDialog
{
	Q_OBJECT

public:
	ImportComicsInfoDialog(QWidget *parent = 0);
	~ImportComicsInfoDialog();
	QString dest;

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
		int progressCount;
		QProgressBar *progressBar;

public slots:
		void findPath();
		void import();
		void close();
};

#endif // IMPORT_COMICS_INFO_DIALOG_H
