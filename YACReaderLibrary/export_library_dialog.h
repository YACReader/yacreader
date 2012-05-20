#ifndef EXPORT_LIBRARY_DIALOG_H
#define EXPORT_LIBRARY_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QThread>
#include <QTimer>

class ExportLibraryDialog : public QDialog 
{
	Q_OBJECT
public:
		ExportLibraryDialog(QWidget * parent = 0);
public slots:
		void exportLibrary();
		void findPath();
		void close();
		void updateProgress();
private:
		int progressCount;
		QLabel * progress;
		QLabel * textLabel;
		QLineEdit * path;
		QPushButton * find;
		QPushButton * accept;
		QPushButton * cancel;
		void run();
		QTimer t;
signals:
		void exportPath(QString);
};

#endif