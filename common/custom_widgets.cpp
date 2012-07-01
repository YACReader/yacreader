#include "custom_widgets.h"

#include <QMessageBox>
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QScrollBar>
#include <QFont>
#include <QMouseEvent>
#include <QStyleOptionViewItem>
#include <QStyleOptionViewItemV4>
#include <QPainter>
#include <QtGlobal>
#include <QDesktopWidget>
#include <QApplication>
#include <QPushButton>
#include <QFileSystemModel>

#include "qnaturalsorting.h"




HelpAboutDialog::HelpAboutDialog(QWidget * parent)
:QDialog(parent)
{
	QVBoxLayout * layout = new QVBoxLayout();

	tabWidget = new QTabWidget();

	tabWidget->addTab(aboutText = new QTextBrowser(), tr("About"));
	aboutText->setOpenExternalLinks(true);
	aboutText->setFont(QFont("Comic Sans MS", 10)); //purisa
	tabWidget->addTab(helpText = new QTextBrowser(), tr("Help"));
	helpText->setOpenExternalLinks(true);
	helpText->setFont(QFont("Comic Sans MS", 10));
	//helpText->setDisabled(true);
	//tabWidget->addTab(,"About Qt");

	layout->addWidget(tabWidget);
	layout->setContentsMargins(1,3,1,1);

	setLayout(layout);
	resize(500, QApplication::desktop()->availableGeometry().height()*0.83);
}

HelpAboutDialog::HelpAboutDialog(const QString & pathAbout,const QString & pathHelp,QWidget * parent)
:QDialog(parent)
{
	loadAboutInformation(pathAbout);
	loadHelp(pathHelp);
}

void HelpAboutDialog::loadAboutInformation(const QString & path)
{
	aboutText->insertHtml(fileToString(path));
	aboutText->moveCursor(QTextCursor::Start);
}

void HelpAboutDialog::loadHelp(const QString & path)
{
	helpText->insertHtml(fileToString(path));
	helpText->moveCursor(QTextCursor::Start);
}

QString HelpAboutDialog::fileToString(const QString & path)
{
	QFile f(path);
	f.open(QIODevice::ReadOnly);
	QTextStream txtS(&f);
	QString content = txtS.readAll();
	f.close();

	return content;
}

void delTree(QDir dir)
{
	dir.setFilter(QDir::AllDirs|QDir::Files|QDir::Hidden|QDir::NoDotAndDotDot);
	QFileInfoList list = dir.entryInfoList();
	for (int i = 0; i < list.size(); ++i)
	{
		QFileInfo fileInfo = list.at(i);
		QString path = fileInfo.filePath();
		if(fileInfo.isDir())
		{
			delTree(QDir(fileInfo.absoluteFilePath()));
			dir.rmdir(fileInfo.absoluteFilePath());
		}
		else
		{
			dir.remove(fileInfo.absoluteFilePath());
		}
	}
}

YACReaderIconProvider::YACReaderIconProvider()
:QFileIconProvider()
{
}

QIcon YACReaderIconProvider::icon(IconType type) const
{
	switch(type)
	{
	case Folder:
		return QIcon(":/images/folder.png");
		break;
	case File:
		return QIcon(":/images/icon.png");
		break;
	default:
		return QFileIconProvider::icon(type);
	}
}
QIcon YACReaderIconProvider::icon(const QFileInfo & info) const
{
	if(info.isDir())
		return QIcon(":/images/folder.png");
	if(info.isFile())
		return QIcon(":/images/icon.png");
}
QString YACReaderIconProvider::type(const QFileInfo & info) const
{
	return  QFileIconProvider::type(info);
}

YACReaderFlow::YACReaderFlow(QWidget * parent,FlowType flowType) : PictureFlow(parent,flowType) {}

void YACReaderFlow::mousePressEvent(QMouseEvent* event)
{
	if(event->x() > (width()+slideSize().width())/2)
		showNext();
	else
		if(event->x() < (width()-slideSize().width())/2)
			showPrevious();
	//else (centered cover space)
}

void YACReaderFlow::mouseDoubleClickEvent(QMouseEvent* event)
{
	if((event->x() > (width()-slideSize().width())/2)&&(event->x() < (width()+slideSize().width())/2))
		emit selected(centerIndex());
}

YACReaderComicDirModel::YACReaderComicDirModel( const QStringList & nameFilters, QDir::Filters filters, QDir::SortFlags sort, QObject * parent )
:QDirModel(nameFilters,filters,sort,parent)
{

}

//this method isn't used to show fileName on QListView
QString YACReaderComicDirModel::fileName ( const QModelIndex & index ) const
{
    QString fileName = QDirModel::fileName(index);
    return fileName.remove(fileName.size()-4,4);
}

QFileInfo YACReaderComicDirModel::fileInfo ( const QModelIndex & index ) const
{
    QFileInfo fileInfo = QDirModel::fileInfo(index);
    QString path = QDir::cleanPath(filePath(index)).remove("/.yacreaderlibrary");
    path.remove(path.size()-4,4);
    fileInfo.setFile(path);
    return fileInfo;
}


YACReaderComicViewDelegate::YACReaderComicViewDelegate(QObject * parent)
:QItemDelegate(parent)
{
}

void YACReaderComicViewDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QStyleOptionViewItemV4 opt = setOptions(index, option);

    const QStyleOptionViewItemV2 *v2 = qstyleoption_cast<const QStyleOptionViewItemV2 *>(&option);
    opt.features = v2 ? v2->features
                    : QStyleOptionViewItemV2::ViewItemFeatures(QStyleOptionViewItemV2::None);
    const QStyleOptionViewItemV3 *v3 = qstyleoption_cast<const QStyleOptionViewItemV3 *>(&option);
    opt.locale = v3 ? v3->locale : QLocale();
    opt.widget = v3 ? v3->widget : 0;

    // prepare
    painter->save();
    painter->setClipRect(opt.rect);

    // get the data and the rectangles

    QVariant value;

    QPixmap pixmap;
    QRect decorationRect;
    QIcon icon;
    value = index.data(Qt::DecorationRole);
    if (value.isValid()) {
        // ### we need the pixmap to call the virtual function
        pixmap = decoration(opt, value);
        if (value.type() == QVariant::Icon) {
            icon = qvariant_cast<QIcon>(value);
            const QSize size = icon.actualSize(option.decorationSize);
            decorationRect = QRect(QPoint(0, 0), size);
        } else {
            icon = QIcon();
            decorationRect = QRect(QPoint(0, 0), pixmap.size());
        }
    } else {
        icon = QIcon();
        decorationRect = QRect();
    }

    QString text;
    QRect displayRect;
    value = index.data(Qt::DisplayRole);
    if (value.isValid() && !value.isNull()) {
        text = value.toString();
        text.remove(text.size()-4,4);
        displayRect = textRectangle(painter, textLayoutBounds(opt), opt.font, text);
    }

    QRect checkRect;
    Qt::CheckState checkState = Qt::Unchecked;
    value = index.data(Qt::CheckStateRole);
    if (value.isValid()) {
        checkState = static_cast<Qt::CheckState>(value.toInt());
        checkRect = check(opt, opt.rect, value);
    }

    // do the layout

    doLayout(opt, &checkRect, &decorationRect, &displayRect, false);

    // draw the item

    drawBackground(painter, opt, index);
    drawCheck(painter, opt, checkRect, checkState);
    drawDecoration(painter, opt, decorationRect, pixmap);
    drawDisplay(painter, opt, displayRect, text);
    drawFocus(painter, opt, displayRect);

    // done
    painter->restore();
}

QRect YACReaderComicViewDelegate::textLayoutBounds(const QStyleOptionViewItemV2 &option) const
{
    QRect rect = option.rect;
    const bool wrapText = option.features & QStyleOptionViewItemV2::WrapText;
    switch (option.decorationPosition) {
    case QStyleOptionViewItem::Left:
    case QStyleOptionViewItem::Right:
        rect.setWidth(wrapText && rect.isValid() ? rect.width() : (1000));
        break;
    case QStyleOptionViewItem::Top:
    case QStyleOptionViewItem::Bottom:
        rect.setWidth(wrapText ? option.decorationSize.width() : (1000));
        break;
    }

    return rect;
}

YACReaderTreeSearch::YACReaderTreeSearch(QObject * parent)
:QSortFilterProxyModel(parent),cache(new ModelIndexCache())
{
	this->setFilterCaseSensitivity(Qt::CaseInsensitive);
}

bool YACReaderTreeSearch::containsFiles(QString path,const QRegExp &exp) const
{
	QDir dir(path);
	QStringList list = dir.entryList(QStringList() << "*"+exp.pattern()+"*",QDir::Files | QDir::NoDotAndDotDot);
	return list.size()>0;
}

bool YACReaderTreeSearch::itemMatchesExpression(const QModelIndex &index, const QRegExp &exp) const
{
	
    QString name = ((QFileSystemModel *)sourceModel())->filePath(index);
	ModelIndexCache::CacheData cd = cache->getCacheData(name);
	bool v = false;
	if(!cd.visited || cd.acepted)
	{
		v = name.contains(exp);// || containsFiles(name,exp); // TODO : complete path?
		int numChildren = sourceModel()->rowCount(index);
		for(int i=0; i<numChildren;i++)
		{
			if(v) break;
			v = v || itemMatchesExpression(sourceModel()->index(i,0,index), exp);
		}
		cd.visited = true;
		cd.acepted = v;
		cache->setModelIndex(name,cd);
	}

	return cd.acepted;
}	

bool YACReaderTreeSearch::filterAcceptsRow ( int sourceRow, const QModelIndex & sourceParent ) const
{
	QString name = sourceModel()->data(sourceModel()->index(sourceRow, 0, sourceParent),Qt::DisplayRole ).toString();
	QFileSystemModel * dm = (QFileSystemModel *)sourceModel();
	if(!dm->isDir(dm->index(sourceRow, 0, sourceParent)))
	//if(name.endsWith(".jpg")||name.endsWith(".db")) //TODO: if is not a dir
		return false;
	if(filterRegExp().isEmpty())
		return true;
	if(name.contains("yacreader"))
		return true;
	QString path = dm->filePath(dm->index(sourceRow, 0, sourceParent));
	if(path.contains("yacreaderlibrary"))
		return itemMatchesExpression(sourceModel()->index(sourceRow, 0, sourceParent), filterRegExp());
	else
		return true;
}

void YACReaderTreeSearch::reset()
{
	//invalidateFilter();
	cache->clear();
}

void YACReaderTreeSearch::softReset()
{
	//invalidateFilter();
}

ModelIndexCache::ModelIndexCache()
:cache()
{
}

void ModelIndexCache::setModelIndex(const QString & index, const CacheData & cd)
{
	cache.insert(index,cd);
}
ModelIndexCache::CacheData ModelIndexCache::getCacheData(const QString & index) const
{
	if(cache.contains(index))
		return cache.value(index);
	else
	{
		CacheData cd;
		cd.visited = false;
		cd.acepted = true;
		return cd;
	}
}

void ModelIndexCache::clear()
{
	cache.clear();
}

YACReaderSortComics::YACReaderSortComics(QObject * parent)
:QSortFilterProxyModel(parent)
{

}

bool YACReaderSortComics::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
	QVariant leftData = sourceModel()->data(left);
	QVariant rightData = sourceModel()->data(right);

	QString leftString = leftData.toString();
	QString rightString = rightData.toString();

	return naturalSortLessThanCI(leftString,rightString);
}


//--------------------------------------------

YACReaderFieldEdit::YACReaderFieldEdit(QWidget * parent)
	:QLineEdit(parent)
{
	setPlaceholderText(tr("Click to overwrite"));
	setModified(false);
	restore = new QAction(tr("Restore to default"),this);
	this->addAction(restore);
	//this->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void YACReaderFieldEdit::focusInEvent(QFocusEvent* e)
{
	if (e->reason() == Qt::MouseFocusReason)
    {
      setModified(true);
	  setPlaceholderText("");
    }

    QLineEdit::focusInEvent(e);
}

void YACReaderFieldEdit::clear()
{
	setPlaceholderText(tr("Click to overwrite"));
	QLineEdit::clear();
	QLineEdit::setModified(false);
}

void YACReaderFieldEdit::setDisabled(bool disabled)
{
	if(disabled)
		setPlaceholderText("");
	QLineEdit::setDisabled(disabled);
}

//--------------------------------------------

YACReaderFieldPlainTextEdit::YACReaderFieldPlainTextEdit(QWidget * parent)
	:QPlainTextEdit(parent)
{
	document()->setModified(false);
	setPlainText(tr("Click to overwrite"));
	restore = new QAction(tr("Restore to default"),this);
	this->addAction(restore);
	//this->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void YACReaderFieldPlainTextEdit::focusInEvent(QFocusEvent* e)
{
	if (e->reason() == Qt::MouseFocusReason  || e->reason() == Qt::TabFocusReason)
    {
      document()->setModified(true);
	  if(toPlainText()==tr("Click to overwrite"))
		setPlainText("");
    }

    QPlainTextEdit::focusInEvent(e);
}

void YACReaderFieldPlainTextEdit::focusOutEvent(QFocusEvent* e)
{
	/*if (e->reason() == Qt::MouseFocusReason  || e->reason() == Qt::TabFocusReason)
	{
		if(toPlainText().isEmpty())
		{
			setPlainText(tr("Click to overwrite"));
			document()->setModified(false);
		}
	}
	*/
	QPlainTextEdit::focusOutEvent(e);
}

void YACReaderFieldPlainTextEdit::clear()
{
	QPlainTextEdit::clear();
	document()->setModified(false);
	setPlainText(tr("Click to overwrite"));
}

void YACReaderFieldPlainTextEdit::setDisabled(bool disabled)
{
	if(disabled)
		setPlainText(tr("Click to overwrite"));
	QPlainTextEdit::setDisabled(disabled);
}