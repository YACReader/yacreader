#ifndef YACREADER_BUSYINDICATOR_H
#define YACREADER_BUSYINDICATOR_H

#include <QTimer>
#include <QWidget>

class YACReaderBusyWidget : public QWidget
{
	Q_OBJECT
public:
	explicit YACReaderBusyWidget(QWidget *parent = 0);
	void paintEvent(QPaintEvent *);
};

class BusyIndicator : public QWidget
{
	Q_OBJECT
public:
	enum IndicatorStyle{StyleRect, StyleEllipse, StyleArc};

	explicit BusyIndicator(QWidget *parent = 0);

	void paintEvent(QPaintEvent *);
	QSize minimumSizeHint() const;
	QSize sizeHint() const;

	void setIndicatorStyle(IndicatorStyle);
	void setColor(QColor color);
	const IndicatorStyle indicatorStyle() const;

signals:

private slots:
	void rotate();

private:
	QPixmap generatePixmap(int sideLength);
	void drawRectStyle(QPainter *painter);
	void drawEllipseStyle(QPainter *painter);
	void drawArcStyle(QPainter *painter);

	QTimer timer;
	int startAngle;

	IndicatorStyle m_style;

	QColor fillColor;
};

#endif // BUSYINDICATOR_H
