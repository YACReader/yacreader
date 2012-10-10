#ifndef __CUSTOM_WIDGETS_H
#define __CUSTOM_WIDGETS_H

#include <QDialog>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QTextBrowser>
#include <QDir>
#include <QFileIconProvider>
#include <QDirModel>
#include <QFileInfo>
#include <QItemDelegate>
#include <QStyleOptionViewItemV2>
#include <QStyleOptionViewItemV4>
#include <QVariant>
#include <QSortFilterProxyModel>
#include <QModelIndex>
#include <QRegExp>
#include <QHash>
#include <QLineEdit>
#include <QAction>
#include <QPlainTextEdit>
#include <QTableView>
#include <QLabel>
#include <QSpinBox>
#include <QSlider>
#include <QRadioButton>

#include "yacreader_global.h"

#include "pictureflow.h"

#include "yacreader_flow_gl.h"

class QToolBarStretch : public QWidget
{
public:
	QToolBarStretch(QWidget * parent=0):QWidget(parent)
	{
		QHBoxLayout * l= new QHBoxLayout();
		l->addStretch();
		setLayout(l);
	}
};

class HelpAboutDialog : public QDialog
{
Q_OBJECT
public:
	HelpAboutDialog(QWidget * parent=0);
	HelpAboutDialog(const QString & pathAbout,const QString & pathHelp,QWidget * parent =0);
public slots:
	void loadAboutInformation(const QString & path);
	void loadHelp(const QString & path);

private:
    QTabWidget *tabWidget;
	QTextBrowser *aboutText;
	QTextBrowser *helpText;
	QString fileToString(const QString & path);
};

class YACReaderIconProvider : public QFileIconProvider
{
public:
    YACReaderIconProvider();
    virtual QIcon icon ( IconType type ) const;
    virtual QIcon icon ( const QFileInfo & info ) const;
    virtual QString type ( const QFileInfo & info ) const;
};

class YACReaderFlow : public PictureFlow
{
Q_OBJECT
public:
        YACReaderFlow(QWidget * parent,FlowType flowType = CoverFlowLike);

	void mousePressEvent(QMouseEvent* event);
	void mouseDoubleClickEvent(QMouseEvent* event);

signals:
	void selected(unsigned int centerIndex);
};

class YACReaderComicDirModel : public QDirModel
{
Q_OBJECT
   public:
    YACReaderComicDirModel( const QStringList & nameFilters, QDir::Filters filters, QDir::SortFlags sort, QObject * parent = 0 );
    QString fileName ( const QModelIndex & index ) const;
    QFileInfo fileInfo ( const QModelIndex & index ) const;
};

class YACReaderComicViewDelegate : public QItemDelegate
{
   Q_OBJECT
   public:
    YACReaderComicViewDelegate(QObject * parent = 0);
    virtual void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    QRect textLayoutBounds(const QStyleOptionViewItemV2 &option) const;
};

class ModelIndexCache
{

public:
		struct CacheData{
		bool visited;
		bool acepted;
	};
	ModelIndexCache();
	void setModelIndex(const QString & index, const CacheData & cd);
	CacheData getCacheData(const QString & index) const;
	void clear();


private:
	 QHash<QString, CacheData> cache;
};


class YACReaderTreeSearch : public QSortFilterProxyModel
{
  Q_OBJECT
   public:
	   YACReaderTreeSearch(QObject * parent = 0);
protected:
	virtual bool filterAcceptsRow ( int sourceRow, const QModelIndex & source_parent ) const;
	bool itemMatchesExpression(const QModelIndex &index, const QRegExp &exp) const;
	bool containsFiles(QString path,const QRegExp &exp) const;
   public slots:
	   void reset();
	   void softReset();
private:
	ModelIndexCache * cache;
};

class YACReaderSortComics : public QSortFilterProxyModel
{
  Q_OBJECT
	public:
		YACReaderSortComics(QObject * parent = 0);
	protected:
		bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

void delTree(QDir dir);


class YACReaderFieldEdit : public QLineEdit
{
  Q_OBJECT
	public:
		YACReaderFieldEdit(QWidget * parent = 0);
		void clear();
		void setDisabled(bool disabled);
	protected:
		void focusInEvent(QFocusEvent* e);
private:
	QAction * restore;

};

class YACReaderFieldPlainTextEdit : public QPlainTextEdit
{
  Q_OBJECT
	public:
		YACReaderFieldPlainTextEdit(QWidget * parent = 0);
		void clear();
		void setDisabled(bool disabled);
	protected:
		void focusInEvent(QFocusEvent* e);
		void focusOutEvent(QFocusEvent* e);
private:
	QAction * restore;

};


//class YACReaderTableView : public QTableView
//{
//    Q_OBJECT
//public:
//    YACReaderTableView(QWidget *parent = 0)
//    :  QTableView(parent)  {}
//
//protected:
//    bool viewportEvent ( QEvent * event )
//    {
//        resizeColumnsToContents();
//        return QTableView::viewportEvent(event);
//    }
//};

class YACReaderSpinSliderWidget : public QWidget
{
	Q_OBJECT
private:
	QLabel * label;
	QSpinBox * spinBox;
	QSlider * slider;
public:
	YACReaderSpinSliderWidget(QWidget * parent = 0);
public slots:
	void setRange(int lowValue, int topValue, int step=1);
	void setValue(int value);
	void setText(const QString & text);
	int getValue();
	QSize minimumSizeHint() const;
signals:
	void valueChanged(int);

};

class YACReaderFlowConfigWidget : public QWidget
{
	Q_OBJECT
public:
	QRadioButton *radio1; 
	QRadioButton *radio2;
	QRadioButton *radio3;

	YACReaderFlowConfigWidget(QWidget * parent = 0);
};

class YACReaderGLFlowConfigWidget : public QWidget
{
	Q_OBJECT
public:
	YACReaderGLFlowConfigWidget(QWidget * parent = 0);

		//GL.........................
		QRadioButton *radioClassic; 
		QRadioButton *radioStripe;
		QRadioButton *radioOver;
		QRadioButton *radionModern;
		QRadioButton *radioDown;

		YACReaderSpinSliderWidget * xRotation;
		YACReaderSpinSliderWidget * yPosition;
		YACReaderSpinSliderWidget * coverDistance;
		YACReaderSpinSliderWidget * centralDistance;
		YACReaderSpinSliderWidget * zoomLevel;
		YACReaderSpinSliderWidget * yCoverOffset;
		YACReaderSpinSliderWidget * zCoverOffset;
		YACReaderSpinSliderWidget * coverRotation;
		YACReaderSpinSliderWidget * fadeOutDist;
		YACReaderSpinSliderWidget * lightStrength;
		YACReaderSpinSliderWidget * maxAngle;

		QSlider * performanceSlider;

public slots:
		void setValues(Preset preset);
};

class YACReaderOptionsDialog : public QDialog
{
	Q_OBJECT
protected:
	YACReaderFlowConfigWidget * sw;
	YACReaderGLFlowConfigWidget * gl;
	QCheckBox * useGL;

	QPushButton * accept;
	QPushButton * cancel;

	QSettings * settings;
	QSettings * previousSettings;

public:
	YACReaderOptionsDialog(QWidget * parent);
public slots:
	virtual void restoreOptions(QSettings * settings);
	virtual void saveOptions();
protected slots:
	virtual void savePerformance(int value);
	virtual void saveUseGL(int b);
	virtual void saveXRotation(int value);
	virtual void saveYPosition(int value);
	virtual void saveCoverDistance(int value);
	virtual void saveCentralDistance(int value);
	virtual void saveZoomLevel(int value);
	virtual void saveYCoverOffset(int value);
	virtual void saveZCoverOffset(int value);
	virtual void saveCoverRotation(int value);
	virtual void saveFadeOutDist(int value);
	virtual void saveLightStrength(int value);
	virtual void saveMaxAngle(int value);
	virtual void loadConfig();
	virtual void setClassicConfig();
	virtual void setStripeConfig();
	virtual void setOverlappedStripeConfig();
	virtual void setModernConfig();
	virtual void setRouletteConfig();

signals:
	void optionsChanged();
};

#endif


