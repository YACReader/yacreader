#include "yacreader_table_view.h"

#include <QHeaderView>
#include <QResizeEvent>
#include <QPropertyAnimation>

#include "yacreader_deleting_progress.h"

YACReaderTableView::YACReaderTableView(QWidget *parent) :
    QTableView(parent),showDelete(false)
{
	setAlternatingRowColors(true);
	verticalHeader()->setAlternatingRowColors(true);
	setStyleSheet("QTableView {alternate-background-color: #F2F2F2;background-color: #FAFAFA; outline: 0px;}"
		"QTableCornerButton::section {background-color:#F5F5F5; border:none; border-bottom:1px solid #B8BDC4; border-right:1px solid qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #D1D1D1, stop: 1 #B8BDC4);}"
		"QTableView::item {outline: 0px; border-bottom: 1px solid #DFDFDF;border-top: 1px solid #FEFEFE; padding-bottom:1px; color:#252626;}"	
		"QTableView::item:selected {outline: 0px; border-bottom: 1px solid #EBCD49;border-top: 1px solid #EBCD49; padding-bottom:1px; background-color: #EBCD49; color: #FFFFFF; }"	
		"QHeaderView::section:horizontal {background-color:#F5F5F5; border-bottom:1px solid #B8BDC4; border-right:1px solid qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #D1D1D1, stop: 1 #B8BDC4); border-left:none; border-top:none; padding:4px; color:#313232;}"
		"QHeaderView::section:vertical {border-bottom: 1px solid #DFDFDF;border-top: 1px solid #FEFEFE;}"
		//"QTableView::item:hover {border-bottom: 1px solid #A3A3A3;border-top: 1px solid #A3A3A3; padding-bottom:1px; background-color: #A3A3A3; color: #FFFFFF; }"
							 "");
	//comicView->setItemDelegate(new YACReaderComicViewDelegate());
	setContextMenuPolicy(Qt::ActionsContextMenu);

	setShowGrid(false);

	verticalHeader()->setResizeMode(QHeaderView::Fixed);

	//comicView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
	horizontalHeader()->setStretchLastSection(true);
	horizontalHeader()->setClickable(false);
	//comicView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
	verticalHeader()->setDefaultSectionSize(24);
	verticalHeader()->setClickable(false); //TODO comportamiento anómalo
	setCornerButtonEnabled(false);

	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::ExtendedSelection);

	/*deletingProgress = new YACReaderDeletingProgress(this);

	showDeletingProgressAnimation = new QPropertyAnimation(deletingProgress,"pos");
	showDeletingProgressAnimation->setDuration(150);*/
}

void YACReaderTableView::showDeleteProgress()
{
	/*showDelete = true;

	showDeletingProgressAnimation->setStartValue(deletingProgress->pos());
	showDeletingProgressAnimation->setEndValue(QPoint((width()-deletingProgress->width())/2 ,1));
	showDeletingProgressAnimation->start();*/
}

void YACReaderTableView::hideDeleteProgress()
{
	/*showDelete = false;

	if(showDeletingProgressAnimation->state()==QPropertyAnimation::Running)
		showDeletingProgressAnimation->stop();

	showDeletingProgressAnimation->setStartValue(deletingProgress->pos());
	showDeletingProgressAnimation->setEndValue(QPoint((width()-deletingProgress->width())/2 ,-deletingProgress->height()));
	showDeletingProgressAnimation->start();*/
}

void YACReaderTableView::resizeEvent(QResizeEvent * event)
{
	/*event->size();

	if(showDelete)
		deletingProgress->move((event->size().width()-deletingProgress->width())/2 ,1);
	else
		deletingProgress->move((event->size().width()-deletingProgress->width())/2 ,-deletingProgress->height());*/

	QTableView::resizeEvent(event);
}
