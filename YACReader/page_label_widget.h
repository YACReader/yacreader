#ifndef PAGE_LABEL_WIDGET_H
#define PAGE_LABEL_WIDGET_H

#include <QWidget>

class QLabel;
class QPropertyAnimation;

class PageLabelWidget : public QWidget
{
Q_OBJECT
private:
	QLabel * textLabel;
	QPropertyAnimation * animation;

protected:
    virtual void paintEvent(QPaintEvent *);

public:
	PageLabelWidget(QWidget * parent);

public slots:
	void show();
	void hide();
	void setText(const QString & text);
	void updatePosition();
};

#endif
