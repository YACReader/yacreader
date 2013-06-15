#ifndef YACREADER_SIDEBAR_H
#define YACREADER_SIDEBAR_H

#include <QWidget>

class YACReaderSideBar : public QWidget
{
    Q_OBJECT
public:
    explicit YACReaderSideBar(QWidget *parent = 0);
	QSize sizeHint() const;
    
signals:
    
public slots:

protected:
	void paintEvent(QPaintEvent *);
    
};

#endif // YACREADER_SIDEBAR_H
