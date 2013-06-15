#include "yacreader_library_list_widget.h"

#include "yacreader_library_item_widget.h"
#include <QVBoxLayout>
#include <QMouseEvent>

YACReaderLibraryListWidget::YACReaderLibraryListWidget(QWidget *parent) :
    QWidget(parent),currentLibraryIndex(0)
{
	QVBoxLayout * mainLayout = new QVBoxLayout;
	mainLayout->setSpacing(0);
	mainLayout->setMargin(0);

	this->setLayout(mainLayout);
}

void YACReaderLibraryListWidget::addItem(QString name, QString path)
{
	QVBoxLayout * mainLayout = dynamic_cast<QVBoxLayout *>(layout());

	YACReaderLibraryItemWidget * library = new YACReaderLibraryItemWidget(name,path,this);
	librariesList.append(library);

	connect(library,SIGNAL(selected(QString,QString)),this,SIGNAL(librarySelected(QString,QString)));
	connect(library,SIGNAL(selected(QString,QString)),this,SLOT(updateLibraries(QString,QString)));

	mainLayout->addWidget(library);

	//first item added
	if(librariesList.count()==1)
	{
		library->select();
		emit currentIndexChanged(name);
	}
}

QString YACReaderLibraryListWidget::currentText()
{
	return librariesList.at(currentLibraryIndex)->name;
}
int YACReaderLibraryListWidget::findText(QString text)
{
	for(int i=0;i<librariesList.count();i++)
	{
		if(librariesList.at(i)->name == text)
			return i;
	}
}
void YACReaderLibraryListWidget::setCurrentIndex(int index)
{
	if(index>=0 && index < librariesList.count())
	{
		librariesList.at(index)->select();
		currentLibraryIndex = index;
		deselectAllBut(index);
	}
}

int YACReaderLibraryListWidget::currentIndex()
{
	return currentLibraryIndex;
}
void YACReaderLibraryListWidget::removeItem(int index)
{
	YACReaderLibraryItemWidget * itemWidget = librariesList.at(index);
	this->layout()->removeWidget(itemWidget);
	librariesList.removeAt(index);
	if(librariesList.count()>0)
	{
		setCurrentIndex(0);
		emit currentIndexChanged(librariesList.at(0)->name);
	}
	delete itemWidget;
}

void YACReaderLibraryListWidget::mousePressEvent ( QMouseEvent * event )
{
	if(librariesList.count()>0)
	{
		int h = librariesList.at(0)->height();
		int item = event->pos().y() / h;
		//deselectAllBut(item);
		setCurrentIndex(item);
		emit currentIndexChanged(librariesList.at(item)->name);

	}
	
}

void YACReaderLibraryListWidget::deselectAllBut(int index)
{
	for(int i=0;i<librariesList.count();i++)
	{
		if(i!=index)
			librariesList.at(i)->deselect();
	}
}