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

#include "pictureflow.h"

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
protected:
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

#endif


