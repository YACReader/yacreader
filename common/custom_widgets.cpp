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
#include <QTextCodec>
#include <QSpinBox>
#include <QLabel>

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

	txtS.setCodec(QTextCodec::codecForName("UTF-8"));

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


YACReaderSpinSliderWidget::YACReaderSpinSliderWidget(QWidget * parent)
	:QWidget(parent)
{
	QHBoxLayout * layout = new QHBoxLayout;
	layout->addWidget(label = new QLabel(this));
	layout->addStretch();
	spinBox = new QSpinBox(this);
	layout->addWidget(spinBox);
	slider = new QSlider(Qt::Horizontal,this);
	layout->addWidget(slider);

	connect(spinBox, SIGNAL(valueChanged(int)), slider,  SLOT(setValue(int)));
	connect(slider,  SIGNAL(valueChanged(int)), spinBox, SLOT(setValue(int)));

	connect(spinBox, SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged(int)));

	setLayout(layout);
}

void YACReaderSpinSliderWidget::setRange(int lowValue, int topValue, int step)
{
	spinBox->setMinimum(lowValue);
	spinBox->setMaximum(topValue);
	spinBox->setSingleStep(step);

	slider->setMinimum(lowValue);
	slider->setMaximum(topValue);
	slider->setSingleStep(step);
}

void YACReaderSpinSliderWidget::setValue(int value)
{
	spinBox->setValue(value);
}

void YACReaderSpinSliderWidget::setText(const QString & text)
{
	label->setText(text);
}

int YACReaderSpinSliderWidget::getValue()
{
	return spinBox->value();
}

QSize YACReaderSpinSliderWidget::minimumSizeHint() const
{
	return QSize(220, 25);
}

//----------------------------------------------------------------------------

YACReaderOptionsDialog::YACReaderOptionsDialog(QWidget * parent)
	:QDialog(parent)
{

	accept = new QPushButton(tr("Save"));
	cancel = new QPushButton(tr("Cancel"));
	connect(accept,SIGNAL(clicked()),this,SLOT(saveOptions()));
	connect(cancel,SIGNAL(clicked()),this,SLOT(restoreOptions()));
	connect(cancel,SIGNAL(clicked()),this,SLOT(close()));

	sw = new YACReaderFlowConfigWidget(this);

	gl = new YACReaderGLFlowConfigWidget(this);
	useGL = useGL = new QCheckBox(tr("Use hardware acceleration (restart needed)"));
	connect(useGL,SIGNAL(stateChanged(int)),this,SLOT(saveUseGL(int)));

	//sw CONNECTIONS
	connect(sw->radio1,SIGNAL(toggled(bool)),this,SLOT(setClassicConfig()));
	connect(sw->radio2,SIGNAL(toggled(bool)),this,SLOT(setStripeConfig()));
	connect(sw->radio3,SIGNAL(toggled(bool)),this,SLOT(setOverlappedStripeConfig()));

	//gl CONNECTIONS
	connect(gl->radioClassic,SIGNAL(toggled(bool)),this,SLOT(setClassicConfig()));
	connect(gl->radioStripe,SIGNAL(toggled(bool)),this,SLOT(setStripeConfig()));
	connect(gl->radioOver,SIGNAL(toggled(bool)),this,SLOT(setOverlappedStripeConfig()));
	connect(gl->radionModern,SIGNAL(toggled(bool)),this,SLOT(setModernConfig()));
	connect(gl->radioDown,SIGNAL(toggled(bool)),this,SLOT(setRouletteConfig()));

	connect(gl->xRotation,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	connect(gl->xRotation,SIGNAL(valueChanged(int)),this,SLOT(saveXRotation(int)));

	connect(gl->yPosition,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	connect(gl->yPosition,SIGNAL(valueChanged(int)),this,SLOT(saveYPosition(int)));

	connect(gl->coverDistance,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	connect(gl->coverDistance,SIGNAL(valueChanged(int)),this,SLOT(saveCoverDistance(int)));

	connect(gl->centralDistance,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	connect(gl->centralDistance,SIGNAL(valueChanged(int)),this,SLOT(saveCentralDistance(int)));

	connect(gl->zoomLevel,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	connect(gl->zoomLevel,SIGNAL(valueChanged(int)),this,SLOT(saveZoomLevel(int)));

	connect(gl->yCoverOffset,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	connect(gl->yCoverOffset,SIGNAL(valueChanged(int)),this,SLOT(saveYCoverOffset(int)));

	connect(gl->zCoverOffset,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	connect(gl->zCoverOffset,SIGNAL(valueChanged(int)),this,SLOT(saveZCoverOffset(int)));

	connect(gl->coverRotation,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	connect(gl->coverRotation,SIGNAL(valueChanged(int)),this,SLOT(saveCoverRotation(int)));

	connect(gl->fadeOutDist,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	connect(gl->fadeOutDist,SIGNAL(valueChanged(int)),this,SLOT(saveFadeOutDist(int)));

	connect(gl->lightStrength,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	connect(gl->lightStrength,SIGNAL(valueChanged(int)),this,SLOT(saveLightStrength(int)));

	connect(gl->maxAngle,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	connect(gl->maxAngle,SIGNAL(valueChanged(int)),this,SLOT(saveMaxAngle(int)));

	connect(gl->performanceSlider, SIGNAL(valueChanged(int)),this,SLOT(savePerformance(int)));
	connect(gl->performanceSlider, SIGNAL(valueChanged(int)),this,SLOT(optionsChanged()));

}

void YACReaderOptionsDialog::savePerformance(int value)
{
	settings->setValue("performance",value);
}

void YACReaderOptionsDialog::saveOptions()
{
	emit(optionsChanged());
	close();
}

void YACReaderOptionsDialog::saveUseGL(int b)
{
	if(Qt::Checked == b)
	{
		sw->setVisible(false);
		gl->setVisible(true);
	}
	else
	{
		gl->setVisible(false);
		sw->setVisible(true);
	}
	resize(0,0);

	settings->setValue("useOpenGL",b);

}

void YACReaderOptionsDialog::saveXRotation(int value)
{
	settings->setValue("flowType",Custom);
	settings->setValue("xRotation",gl->xRotation->getValue());
}
void YACReaderOptionsDialog::saveYPosition(int value)
{
	settings->setValue("flowType",Custom);
	settings->setValue("yPosition",gl->yPosition->getValue());
}
void YACReaderOptionsDialog::saveCoverDistance(int value)
{
	settings->setValue("flowType",Custom);
	settings->setValue("coverDistance",gl->coverDistance->getValue());
}
void YACReaderOptionsDialog::saveCentralDistance(int value)
{
	settings->setValue("flowType",Custom);
	settings->setValue("centralDistance",gl->centralDistance->getValue());
}
void YACReaderOptionsDialog::saveZoomLevel(int value)
{
	settings->setValue("flowType",Custom);
	settings->setValue("zoomLevel",gl->zoomLevel->getValue());
}
void YACReaderOptionsDialog::saveYCoverOffset(int value)
{
	settings->setValue("flowType",Custom);
	settings->setValue("yCoverOffset",gl->yCoverOffset->getValue());
}
void YACReaderOptionsDialog::saveZCoverOffset(int value)
{
	settings->setValue("flowType",Custom);
	settings->setValue("zCoverOffset",gl->zCoverOffset->getValue());
}
void YACReaderOptionsDialog::saveCoverRotation(int value)
{
	settings->setValue("flowType",Custom);
	settings->setValue("coverRotation",gl->coverRotation->getValue());
}
void YACReaderOptionsDialog::saveFadeOutDist(int value)
{
	settings->setValue("flowType",Custom);
	settings->setValue("fadeOutDist",gl->fadeOutDist->getValue());
}
void YACReaderOptionsDialog::saveLightStrength(int value)
{
	settings->setValue("flowType",Custom);
	settings->setValue("lightStrength",gl->lightStrength->getValue());
}

void YACReaderOptionsDialog::saveMaxAngle(int value)
{
	settings->setValue("flowType",Custom);
	settings->setValue("maxAngle",gl->maxAngle->getValue());
}

void YACReaderOptionsDialog::restoreOptions(QSettings * settings)
{
	this->settings = settings;

	//FLOW CONFIG

	if(settings->contains("useOpenGL") && settings->value("useOpenGL").toInt() == Qt::Checked)
	{
		sw->setVisible(false);
		gl->setVisible(true);
		useGL->setChecked(true);
	}
	else
	{
		gl->setVisible(false);
		sw->setVisible(true);
		useGL->setChecked(false);
	}
		

	if(!settings->contains("flowType"))
	{
		setClassicConfig();
		gl->radioClassic->setChecked(true);
		gl->performanceSlider->setValue(1);
		return;
	}

	gl->performanceSlider->setValue(settings->value("performance").toInt());
	FlowType flowType;
	switch(settings->value("flowType").toInt())
	{
	case 0:
		flowType = CoverFlowLike;
		break;
	case 1:
		flowType = Strip;
		break;
	case 2:
		flowType = StripOverlapped;
		break;
	case 3:
		flowType = Modern;
		break;
	case 4:
		flowType = Roulette;
		break;
	case 5:
		flowType = Custom;
		break;
	}
	

	if(flowType == Custom)
	{
		loadConfig();
		return;
	}

	if(flowType == CoverFlowLike)
	{
		setClassicConfig();
		gl->radioClassic->setChecked(true);
		return;
	}

	if(flowType == Strip)
	{
		setStripeConfig();
		gl->radioStripe->setChecked(true);
		return;
	}

	if(flowType == StripOverlapped)
	{
		setOverlappedStripeConfig();
		gl->radioOver->setChecked(true);
		return;
	}

	if(flowType == Modern)
	{
		setModernConfig();
		gl->radionModern->setChecked(true);
		return;
	}
	
	if(flowType == Roulette)
	{
		setRouletteConfig();
		gl->radioDown->setChecked(true);
		return;
	}

	//END FLOW CONFIG
}

void YACReaderOptionsDialog::loadConfig()
{
	gl->xRotation->setValue(settings->value("xRotation").toInt());
	gl->yPosition->setValue(settings->value("yPosition").toInt());
	gl->coverDistance->setValue(settings->value("coverDistance").toInt());
	gl->centralDistance->setValue(settings->value("centralDistance").toInt());
	gl->zoomLevel->setValue(settings->value("zoomLevel").toInt());
	gl->yCoverOffset->setValue(settings->value("yCoverOffset").toInt());
	gl->zCoverOffset->setValue(settings->value("zCoverOffset").toInt());
	gl->coverRotation->setValue(settings->value("coverRotation").toInt());
	gl->fadeOutDist->setValue(settings->value("fadeOutDist").toInt());
	gl->lightStrength->setValue(settings->value("lightStrength").toInt());
	gl->maxAngle->setValue(settings->value("maxAngle").toInt());
}
void YACReaderOptionsDialog::setClassicConfig()
{
	settings->setValue("flowType",CoverFlowLike);

	gl->setValues(presetYACReaderFlowClassicConfig);
}

void YACReaderOptionsDialog::setStripeConfig()
{
	settings->setValue("flowType",Strip);

	gl->setValues(presetYACReaderFlowStripeConfig);
}

void YACReaderOptionsDialog::setOverlappedStripeConfig()
{
	settings->setValue("flowType",StripOverlapped);

	gl->setValues(presetYACReaderFlowOverlappedStripeConfig);
}

void YACReaderOptionsDialog::setModernConfig()
{
	settings->setValue("flowType",Modern);

	gl->setValues(defaultYACReaderFlowConfig);
}

void YACReaderOptionsDialog::setRouletteConfig()
{
	settings->setValue("flowType",Roulette);

	gl->setValues(pressetYACReaderFlowDownConfig);
}


///----------------------------------------------------------
YACReaderGLFlowConfigWidget::YACReaderGLFlowConfigWidget(QWidget * parent /* = 0 */)
	:QWidget(parent)
{
	QVBoxLayout * layout = new QVBoxLayout(this);

	//PRESETS------------------------------------------------------------------
	QGroupBox *groupBox = new QGroupBox(tr("Presets:"));

	radioClassic = new QRadioButton(tr("Classic look"));
	connect(radioClassic,SIGNAL(toggled(bool)),this,SLOT(setClassicConfig()));

	radioStripe  = new QRadioButton(tr("Stripe look"));
	connect(radioStripe,SIGNAL(toggled(bool)),this,SLOT(setStripeConfig()));

	radioOver    = new QRadioButton(tr("Overlapped Stripe look"));
	connect(radioOver,SIGNAL(toggled(bool)),this,SLOT(setOverlappedStripeConfig()));

	radionModern = new QRadioButton(tr("Modern look"));
	connect(radionModern,SIGNAL(toggled(bool)),this,SLOT(setModernConfig()));

	radioDown    = new QRadioButton(tr("Roulette look"));
	connect(radioDown,SIGNAL(toggled(bool)),this,SLOT(setRouletteConfig()));

	QVBoxLayout *vbox = new QVBoxLayout;
	QHBoxLayout * opt1 = new QHBoxLayout;
	opt1->addWidget(radioClassic);
	QLabel * lOpt1 = new QLabel();
	lOpt1->setPixmap(QPixmap(":/images/flow1.png"));
	opt1->addStretch();
	opt1->addWidget(lOpt1);
	vbox->addLayout(opt1);

	QHBoxLayout * opt2 = new QHBoxLayout;
	opt2->addWidget(radioStripe);
	QLabel * lOpt2 = new QLabel();
	lOpt2->setPixmap(QPixmap(":/images/flow2.png"));
	opt2->addStretch();
	opt2->addWidget(lOpt2);
	vbox->addLayout(opt2);

	QHBoxLayout * opt3 = new QHBoxLayout;
	opt3->addWidget(radioOver);
	QLabel * lOpt3 = new QLabel();
	lOpt3->setPixmap(QPixmap(":/images/flow3.png"));
	opt3->addStretch();
	opt3->addWidget(lOpt3);
	vbox->addLayout(opt3);

	QHBoxLayout * opt4 = new QHBoxLayout;
	opt4->addWidget(radionModern);
	QLabel * lOpt4 = new QLabel();
	lOpt4->setPixmap(QPixmap(":/images/flow3.png"));
	opt4->addStretch();
	opt4->addWidget(lOpt4);
	vbox->addLayout(opt4);

	QHBoxLayout * opt5 = new QHBoxLayout;
	opt5->addWidget(radioDown);
	QLabel * lOpt5 = new QLabel();
	lOpt5->setPixmap(QPixmap(":/images/flow3.png"));
	opt5->addStretch();
	opt5->addWidget(lOpt5);
	vbox->addLayout(opt5);
	
	groupBox->setLayout(vbox);

	//OPTIONS------------------------------------------------------------------
	QGroupBox *optionsGroupBox = new QGroupBox(tr("Custom:"));

	xRotation = new YACReaderSpinSliderWidget(this);
	xRotation->setText(tr("View angle"));
	xRotation->setRange(0,90);
	//connect(xRotation,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	//connect(xRotation,SIGNAL(valueChanged(int)),this,SLOT(saveXRotation(int)));

	yPosition = new YACReaderSpinSliderWidget(this);
	yPosition->setText(tr("Position"));
	yPosition->setRange(-100,100);
	//connect(yPosition,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	//connect(yPosition,SIGNAL(valueChanged(int)),this,SLOT(saveYPosition(int)));

	coverDistance = new YACReaderSpinSliderWidget(this);
	coverDistance->setText(tr("Cover gap"));
	coverDistance->setRange(0,150);
	//connect(coverDistance,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	//connect(coverDistance,SIGNAL(valueChanged(int)),this,SLOT(saveCoverDistance(int)));

	centralDistance = new YACReaderSpinSliderWidget(this);
	centralDistance->setText(tr("Central gap"));
	centralDistance->setRange(0,150);
	//connect(centralDistance,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	//connect(centralDistance,SIGNAL(valueChanged(int)),this,SLOT(saveCentralDistance(int)));

	zoomLevel = new YACReaderSpinSliderWidget(this);
	zoomLevel->setText(tr("Zoom"));
	zoomLevel->setRange(-20,0);
	//connect(zoomLevel,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	//connect(zoomLevel,SIGNAL(valueChanged(int)),this,SLOT(saveZoomLevel(int)));

	yCoverOffset = new YACReaderSpinSliderWidget(this);
	yCoverOffset->setText(tr("Y offset"));
	yCoverOffset->setRange(-50,50);
	//connect(yCoverOffset,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	//connect(yCoverOffset,SIGNAL(valueChanged(int)),this,SLOT(saveYCoverOffset(int)));

	zCoverOffset = new YACReaderSpinSliderWidget(this);
	zCoverOffset->setText(tr("Z offset"));
	zCoverOffset->setRange(-50,50);
	//connect(zCoverOffset,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	//connect(zCoverOffset,SIGNAL(valueChanged(int)),this,SLOT(saveZCoverOffset(int)));

	coverRotation = new YACReaderSpinSliderWidget(this);
	coverRotation->setText(tr("Cover Angle"));
	coverRotation->setRange(0,360);
	//connect(coverRotation,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	//connect(coverRotation,SIGNAL(valueChanged(int)),this,SLOT(saveCoverRotation(int)));

	fadeOutDist = new YACReaderSpinSliderWidget(this);
	fadeOutDist->setText(tr("Visibility"));
	fadeOutDist->setRange(0,10);
	//connect(fadeOutDist,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	//connect(fadeOutDist,SIGNAL(valueChanged(int)),this,SLOT(saveFadeOutDist(int)));

	lightStrength = new YACReaderSpinSliderWidget(this);
	lightStrength->setText(tr("Light"));
	lightStrength->setRange(-10,10);
	//connect(lightStrength,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	//connect(lightStrength,SIGNAL(valueChanged(int)),this,SLOT(saveLightStrength(int)));

	maxAngle = new YACReaderSpinSliderWidget(this);
	maxAngle->setText(tr("Max angle"));
	maxAngle->setRange(0,90);
	//connect(maxAngle,SIGNAL(valueChanged(int)),this,SIGNAL(optionsChanged()));
	//connect(maxAngle,SIGNAL(valueChanged(int)),this,SLOT(saveMaxAngle(int)));

	QVBoxLayout *optionsLayoutStretch = new QVBoxLayout;
	optionsLayoutStretch->setContentsMargins(0,0,0,0);
	QGridLayout *optionsLayout = new QGridLayout;
	optionsLayout->addWidget(xRotation,0,0);
	optionsLayout->addWidget(yPosition,0,1);
	optionsLayout->addWidget(coverDistance,1,0);
	optionsLayout->addWidget(centralDistance,1,1);
	optionsLayout->addWidget(zoomLevel,2,0);
	optionsLayout->addWidget(yCoverOffset,2,1);
	optionsLayout->addWidget(zCoverOffset,3,0);
	optionsLayout->addWidget(coverRotation,3,1);
	optionsLayout->addWidget(fadeOutDist,4,0);
	optionsLayout->addWidget(lightStrength,4,1);
	optionsLayout->addWidget(maxAngle,5,0);
	
	optionsLayoutStretch->addLayout(optionsLayout);
	optionsLayoutStretch->addStretch();

	optionsGroupBox->setLayout(optionsLayoutStretch);

	QHBoxLayout * groupBoxesLayout = new QHBoxLayout;
	groupBoxesLayout->addWidget(groupBox);
	groupBoxesLayout->addWidget(optionsGroupBox);

	QHBoxLayout * performance = new QHBoxLayout;
	performance->addWidget(new QLabel(tr("Low Performance")));
	performance->addWidget(performanceSlider = new QSlider(Qt::Horizontal));
	performance->addWidget(new QLabel(tr("High Performance")));

	performanceSlider->setMinimum(0);
	performanceSlider->setMaximum(3);
	performanceSlider->setSingleStep(1);
	performanceSlider->setPageStep(1);
	performanceSlider->setTickInterval(1);
	performanceSlider->setTickPosition(QSlider::TicksRight);

	//connect(performanceSlider, SIGNAL(valueChanged(int)),this,SLOT(savePerformance(int)));
	//connect(performanceSlider, SIGNAL(valueChanged(int)),this,SLOT(optionsChanged()));


	layout->addLayout(groupBoxesLayout);
	layout->addLayout(performance);

	layout->setContentsMargins(0,0,0,0);

	setLayout(layout);


}

void YACReaderGLFlowConfigWidget::setValues(Preset preset)
{
	xRotation->setValue(preset.cfRX);
	yPosition->setValue(preset.cfY*100);
	coverDistance->setValue(preset.xDistance*100);
	centralDistance->setValue(preset.centerDistance*100);
	zoomLevel->setValue(preset.cfZ);
	yCoverOffset->setValue(preset.yDistance*100);
	zCoverOffset->setValue(preset.zDistance*100);
	coverRotation->setValue(preset.rotation*-1);
	fadeOutDist->setValue(preset.animationFadeOutDist);
	lightStrength->setValue(preset.viewRotateLightStrenght);
	maxAngle->setValue(preset.viewAngle);
}

//-----------------------------------------------------------------------------
YACReaderFlowConfigWidget::YACReaderFlowConfigWidget(QWidget * parent )
	:QWidget(parent)
{
	QVBoxLayout * layout = new QVBoxLayout(this);

	QGroupBox *groupBox = new QGroupBox(tr("How to show covers:"));

	radio1 = new QRadioButton(tr("CoverFlow look"));
	radio2 = new QRadioButton(tr("Stripe look"));
	radio3 = new QRadioButton(tr("Overlapped Stripe look"));


	QVBoxLayout *vbox = new QVBoxLayout;
	QHBoxLayout * opt1 = new QHBoxLayout;
	opt1->addWidget(radio1);
	QLabel * lOpt1 = new QLabel();
	lOpt1->setPixmap(QPixmap(":/images/flow1.png"));
	opt1->addStretch();
	opt1->addWidget(lOpt1);
	vbox->addLayout(opt1);

	QHBoxLayout * opt2 = new QHBoxLayout;
	opt2->addWidget(radio2);
	QLabel * lOpt2 = new QLabel();
	lOpt2->setPixmap(QPixmap(":/images/flow2.png"));
	opt2->addStretch();
	opt2->addWidget(lOpt2);
	vbox->addLayout(opt2);

	QHBoxLayout * opt3 = new QHBoxLayout;
	opt3->addWidget(radio3);
	QLabel * lOpt3 = new QLabel();
	lOpt3->setPixmap(QPixmap(":/images/flow3.png"));
	opt3->addStretch();
	opt3->addWidget(lOpt3);
	vbox->addLayout(opt3);


	//vbox->addStretch(1);
	groupBox->setLayout(vbox);

	layout->addWidget(groupBox);

	layout->setContentsMargins(0,0,0,0);

	setLayout(layout);
}