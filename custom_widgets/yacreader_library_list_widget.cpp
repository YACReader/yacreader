#include "yacreader_library_list_widget.h"

#include "yacreader_library_item_widget.h"
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QMenu>
#include "qnaturalsorting.h"

YACReaderLibraryListWidget::YACReaderLibraryListWidget(QWidget *parent) :
    QWidget(parent),currentLibraryIndex(-1)
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
	connect(library,SIGNAL(showOptions()),this,SLOT(showContextMenu()));
	QList<YACReaderLibraryItemWidget *>::iterator itr;
	int i = 0;
	for(itr = librariesList.begin(); itr!=librariesList.end() && !naturalSortLessThanCI(name,(*itr)->name);itr++)
		i++;

	librariesList.insert(itr,library);

	//connect(library,SIGNAL(selected(QString,QString)),this,SIGNAL(librarySelected(QString,QString)));
	connect(library,SIGNAL(selected(QString,QString)),this,SLOT(updateLibraries(QString,QString)));

	mainLayout->insertWidget(i,library);
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
		emit currentIndexChanged(librariesList.at(currentLibraryIndex)->name);
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
	}
	delete itemWidget;
}

void YACReaderLibraryListWidget::mousePressEvent ( QMouseEvent * event )
{
	if(librariesList.count()>0)
	{
		int h = librariesList.at(0)->height();
		int item = event->pos().y() / h;
		if(item!=currentLibraryIndex)
		{
			setCurrentIndex(item);
		}
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

void YACReaderLibraryListWidget::showContextMenu()
{
	YACReaderLibraryItemWidget * itemWidget = librariesList.at(currentLibraryIndex);
	QMenu::exec(actions(),itemWidget->mapToGlobal(QPoint(itemWidget->width()-8,itemWidget->height()/2)));
}