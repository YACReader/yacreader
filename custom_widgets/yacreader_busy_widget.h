#ifndef YACREADER_BUSYINDICATOR_H
#define YACREADER_BUSYINDICATOR_H

#include <QTimer>
#include <QWidget>

class YACReaderBusyWidget : public QWidget
{
    Q_OBJECT
public:
    explicit YACReaderBusyWidget(QWidget *parent = 0);
};

class BusyIndicator : public QWidget
{
    Q_OBJECT
public:
    enum IndicatorStyle { StyleRect,
                          StyleEllipse,
                          StyleArc };

    explicit BusyIndicator(QWidget *parent = 0, int size = 30);

    void paintEvent(QPaintEvent *) override;
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    void setIndicatorStyle(IndicatorStyle);
    void setColor(QColor color);
    IndicatorStyle indicatorStyle() const;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void clicked();

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
    int size;
};

#endif // BUSYINDICATOR_H
