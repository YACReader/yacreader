#ifndef YACREADER_LIBRARY_LIST_WIDGET_H
#define YACREADER_LIBRARY_LIST_WIDGET_H

#include <QWidget>

class YACReaderLibraryItemWidget;
class QMouseEvent;

class YACReaderLibraryListWidget : public QWidget
{
	Q_OBJECT
public:
	explicit YACReaderLibraryListWidget(QWidget *parent = 0);
	
signals:
	void currentIndexChanged(QString text);
	
public slots:
	QString currentText();
	int findText(QString text);
	void setCurrentIndex(int index);
	void addItem(QString name, QString path);
	int currentIndex();
	void removeItem(int index);
	void showContextMenu();
	void renameCurrentLibrary(QString newName);
protected:
	void mousePressEvent ( QMouseEvent * event );
private:
	int currentLibraryIndex;
	QList < YACReaderLibraryItemWidget* > librariesList;
	void deselectAllBut(int index);
	
};

#endif // YACREADER_LIBRARY_LIST_WIDGET_H

