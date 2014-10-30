#include "yacreader_table_view.h"

#include <QHeaderView>
#include <QResizeEvent>
#include <QPropertyAnimation>
#include <QPainter>

#include "comic_item.h"

YACReaderTableView::YACReaderTableView(QWidget *parent) :
	QTableView(parent),showDelete(false),editing(false),myeditor(0)
{
	setAlternatingRowColors(true);
	verticalHeader()->setAlternatingRowColors(true);
	setStyleSheet("QTableView {alternate-background-color: #F2F2F2;background-color: #FAFAFA; outline: 0px;}"// border: 1px solid #999999; border-right:none; border-bottom:none;}"
		"QTableCornerButton::section {background-color:#F5F5F5; border:none; border-bottom:1px solid #B8BDC4; border-right:1px solid qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #D1D1D1, stop: 1 #B8BDC4);}"
		"QTableView::item {outline: 0px; border-bottom: 1px solid #DFDFDF;border-top: 1px solid #FEFEFE; padding-bottom:1px; color:#252626;}"	
		"QTableView {border-top:1px solid #B8B8B8;border-bottom:none;border-left:1px solid #B8B8B8;border-right:none;}"
#ifdef Q_OS_MAC
		"QTableView {border-top:1px solid #B8B8B8;border-bottom:none;border-left:none;border-right:none;}"          
		"QTableView::item:selected {outline: 0px; border-bottom: 1px solid #3875D7;border-top: 1px solid #3875D7; padding-bottom:1px; background-color: #3875D7; color: #FFFFFF; }"

#else
		"QTableView::item:selected {outline: 0px; border-bottom: 1px solid #D4D4D4;border-top: 1px solid #D4D4D4; padding-bottom:1px; background-color: #D4D4D4;  }"
#endif
				  "QHeaderView::section:horizontal {background-color:#F5F5F5; border-bottom:1px solid #B8BDC4; border-right:1px solid qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #D1D1D1, stop: 1 #B8BDC4); border-left:none; border-top:none; padding:4px; color:#313232;}"
		"QHeaderView::section:vertical {border-bottom: 1px solid #DFDFDF;border-top: 1px solid #FEFEFE;}"
		//"QTableView::item:hover {border-bottom: 1px solid #A3A3A3;border-top: 1px solid #A3A3A3; padding-bottom:1px; background-color: #A3A3A3; color: #FFFFFF; }"
							 "");
	//comicView->setItemDelegate(new YACReaderComicViewDelegate());
	setContextMenuPolicy(Qt::ActionsContextMenu);

	setShowGrid(false);
#if QT_VERSION >= 0x050000
	verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
#else
	verticalHeader()->setResizeMode(QHeaderView::Fixed);
#endif

	//comicView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
	horizontalHeader()->setStretchLastSection(true);
#if QT_VERSION >= 0x050000
	horizontalHeader()->setSectionsClickable(false);
#else
	horizontalHeader()->setClickable(false);
#endif
	//comicView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
	verticalHeader()->setDefaultSectionSize(24);
#if QT_VERSION >= 0x050000
	verticalHeader()->setSectionsClickable(false); //TODO comportamiento anómalo
#else
	verticalHeader()->setClickable(false); //TODO comportamiento anómalo
#endif

	setCornerButtonEnabled(false);

	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::ExtendedSelection);

	setItemDelegateForColumn(11,new YACReaderRatingDelegate(this));
	setEditTriggers(QAbstractItemView::NoEditTriggers);

	setMouseTracking(true);
	/*deletingProgress = new YACReaderDeletingProgress(this);

	showDeletingProgressAnimation = new QPropertyAnimation(deletingProgress,"pos");
	showDeletingProgressAnimation->setDuration(150);*/
}

void YACReaderTableView::mouseMoveEvent(QMouseEvent *event)
{
	QModelIndex mi = indexAt(event->pos());
	if(mi.isValid())
	{
		QList<QModelIndex> selectedIndexes = this->selectedIndexes();
		if(selectedIndexes.contains(mi))
		{
			if(mi.column() == 11)
			{
				if(!editing)
				{
					editing = true;
					currentIndexEditing = mi;
					edit(mi);
					myeditor = indexWidget(mi);
				}
				else if(mi.row() != currentIndexEditing.row())
					closeRatingEditor();
			}
			else
				closeRatingEditor();
		}
		else
			closeRatingEditor();
	}
	else
		closeRatingEditor();

	QTableView::mouseMoveEvent(event);
}
void YACReaderTableView::mousePressEvent(QMouseEvent * event)
{
	QTableView::mousePressEvent(event);
	QModelIndex mi = indexAt(event->pos());
	if(mi.isValid())
	{
		QList<QModelIndex> selectedIndexes = this->selectedIndexes();
		if(selectedIndexes.contains(mi))
		{
			if(mi.column() == 11)
			{
				if(!editing)
				{
					editing = true;
					currentIndexEditing = mi;
					edit(mi);
					myeditor = indexWidget(mi);
				}
			}
		}
	}
}
void YACReaderTableView::leaveEvent(QEvent * event)
{
	closeRatingEditor();
	event->accept();
}

void YACReaderTableView::closeRatingEditor()
{
	editing = false;
	if(myeditor!=0)
		closeEditor(myeditor,QAbstractItemDelegate::NoHint);
	myeditor = 0;
}

void YACReaderTableView::closeEditor ( QWidget * editor, QAbstractItemDelegate::EndEditHint hint )
{
	editing = false;
	myeditor = 0;
	QTableView::closeEditor(editor,hint);
}
void YACReaderTableView::commitData ( QWidget * editor )
{
	//TODO
	StarEditor *starEditor = qobject_cast<StarEditor *>(editor);
	if(starEditor->getShouldCommitData())
		emit comicRated(((StarEditor *)editor)->starRating().starCount(),currentIndexEditing);
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

//------------------------------------------------------------------------------
//YACReaderRatingDelegate-------------------------------------------------------
//------------------------------------------------------------------------------
void YACReaderRatingDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
									const QModelIndex &index) const
{
    int rating = ((ComicItem *)index.internalPointer())->data(11).toInt();

	StarRating starRating(rating);

	QStyledItemDelegate::paint(painter, option, index);

	if(!(option.state & QStyle::State_Editing))
	{
		if (option.state & QStyle::State_Selected)
			starRating.paintSelected(painter, option.rect, option.palette,
			StarRating::ReadOnly);
		else
			starRating.paint(painter, option.rect, option.palette,
			StarRating::ReadOnly);
	}
}

QSize YACReaderRatingDelegate::sizeHint(const QStyleOptionViewItem &option,
							 const QModelIndex &index) const
{
	Q_UNUSED(option)
    int rating = ((ComicItem *)index.internalPointer())->data(11).toInt();
	StarRating starRating(rating);
	return starRating.sizeHint();
}

QWidget *YACReaderRatingDelegate::createEditor(QWidget *parent,
									const QStyleOptionViewItem &option,
									const QModelIndex &index) const
{
	Q_UNUSED(option)
	Q_UNUSED(index)
	StarEditor *editor = new StarEditor(parent);
	connect(editor, SIGNAL(editingFinished()),
		this, SLOT(sendCloseEditor()));
	connect(editor, SIGNAL(commitData()),
		this, SLOT(sendCommitData()));
	return editor;
}

void YACReaderRatingDelegate::setEditorData(QWidget *editor,
								 const QModelIndex &index) const
{
    int rating = ((ComicItem *)index.internalPointer())->data(11).toInt();

	StarRating starRating(rating);

	StarEditor *starEditor = qobject_cast<StarEditor *>(editor);
	starEditor->setStarRating(starRating);
}

void YACReaderRatingDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
								const QModelIndex &index) const
{
	QStyledItemDelegate::setModelData(editor, model, index);
}

void YACReaderRatingDelegate::sendCommitData()
{
	StarEditor *editor = qobject_cast<StarEditor *>(sender());
	emit commitData(editor);
}
void YACReaderRatingDelegate::sendCloseEditor()
{
	StarEditor *editor = qobject_cast<StarEditor *>(sender());
	emit closeEditor(editor);
}

//-------------------------------------------------------------------------------
//StarRating---------------------------------------------------------------------
//-------------------------------------------------------------------------------

const int PaintingScaleFactor = 20;

StarRating::StarRating(int starCount, int maxStarCount)
{
	myStarCount = starCount;
	myMaxStarCount = maxStarCount;

	int numVertex = 5;
	double pi = 3.14159265359;
	double angle = 3.14159265359 / numVertex;

	float rOuter = 0.3f;
	float rInner = 0.15f;
	for (int i = 0; i < 2 * numVertex; i++)
	{
		double r = (i & 1) == 0 ? rOuter : rInner;
		starPolygon << QPointF(0.5 + cos((i * angle)-pi/2) * r, 0.5 + sin((i * angle)-pi/2) * r);
	}

	diamondPolygon << QPointF(0.4, 0.5) << QPointF(0.5, 0.4)
				   << QPointF(0.6, 0.5) << QPointF(0.5, 0.6)
				   << QPointF(0.4, 0.5);
}

QSize StarRating::sizeHint() const
{
	return PaintingScaleFactor * QSize(myMaxStarCount, 1);
}

void StarRating::paint(QPainter *painter, const QRect &rect,
					   const QPalette &palette, EditMode mode) const
{
	Q_UNUSED(palette)
	painter->save();

	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->setPen(Qt::NoPen);

	//if (mode == Editable) {
	//    painter->setBrush(palette.highlight());
	//} else {
	QBrush brush(QColor("#e9be0f"));
	painter->setBrush(brush);
	//}

	int yOffset = (rect.height() - PaintingScaleFactor) / 2;
	painter->translate(rect.x(), rect.y() + yOffset);
	painter->scale(PaintingScaleFactor, PaintingScaleFactor);

	for (int i = 0; i < myMaxStarCount; ++i) {
		if (i < myStarCount) {
			painter->drawPolygon(starPolygon, Qt::WindingFill);
		} else if (mode == Editable) {
			painter->drawEllipse(QPointF(0.5,0.5),0.08,0.08);//(diamondPolygon, Qt::WindingFill);
		}
		painter->translate(1.0, 0.0);
	}

	painter->restore();
}

void StarRating::paintSelected(QPainter *painter, const QRect &rect,
					   const QPalette &palette, EditMode mode, QColor color) const
{
	Q_UNUSED(palette)
	Q_UNUSED(mode)
	painter->save();

	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->setPen(Qt::NoPen);

	QBrush star(color);
	QBrush dot(QColor("#ffffff"));

	int yOffset = (rect.height() - PaintingScaleFactor) / 2;
	painter->translate(rect.x(), rect.y() + yOffset);
	painter->scale(PaintingScaleFactor, PaintingScaleFactor);

	for (int i = 0; i < myMaxStarCount; ++i) {
		if (i < myStarCount) {
			painter->setBrush(star);
			painter->drawPolygon(starPolygon, Qt::WindingFill);
		} else {
			painter->setBrush(dot);
			painter->drawEllipse(QPointF(0.5,0.5),0.08,0.08);
		}
		painter->translate(1.0, 0.0);
	}

	painter->restore();
}

void StarRating::paintSelected(QPainter *painter, const QRect &rect,
					   const QPalette &palette, EditMode mode) const
{
	paintSelected(painter,rect, palette,mode,QColor("#ffffff"));
}


//-------------------------------------------------------------------------------
//StarEditor---------------------------------------------------------------------
//-------------------------------------------------------------------------------

StarEditor::StarEditor(QWidget *parent)
	: QWidget(parent),shouldCommitData(false)
{
	//setMouseTracking(true);
	//setAutoFillBackground(true);
}

QSize StarEditor::sizeHint() const
{
	return myStarRating.sizeHint();
}

void StarEditor::paintEvent(QPaintEvent *)
{
	/*
	QPainter painter(this);
	myStarRating.paintSelected(&painter, rect(), this->palette(),
		StarRating::Editable,QColor("#615f59"));*/
}

void StarEditor::mouseMoveEvent(QMouseEvent *event)
{
	Q_UNUSED(event)
	/*int star = starAtPosition(event->x());

	if (star != myStarRating.starCount() && star != -1) {
		myStarRating.setStarCount(star);
		update();
	}*/
}
void StarEditor::leaveEvent(QEvent * event){
	emit editingFinished();
	QWidget::leaveEvent(event);
}

void StarEditor::mousePressEvent(QMouseEvent *  event )
{
    if(event->button() == Qt::LeftButton)
    {
        int star = starAtPosition(event->x());

        if (star != myStarRating.starCount() && star != -1) {
            myStarRating.setStarCount(star);
            shouldCommitData = true;
            emit commitData();
        }
    }
}

int StarEditor::starAtPosition(int x)
{
	int star = (x / (myStarRating.sizeHint().width()
					 / myStarRating.maxStarCount())) + 1;
	if (star <= 0 || star > myStarRating.maxStarCount())
		return -1;

	return star;
}
