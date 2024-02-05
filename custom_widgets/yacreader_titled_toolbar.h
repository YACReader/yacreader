#ifndef YACREADER_TITLED_TOOLBAR_H
#define YACREADER_TITLED_TOOLBAR_H

#include <QWidget>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QPoint>

class QIcon;
class BusyIndicator;

class DropShadowLabel : public QLabel
{
    Q_OBJECT

public:
    DropShadowLabel(QWidget *parent = 0);
    void paintEvent(QPaintEvent *event);
    void setColor(const QColor &color);
    void setDropShadowColor(const QColor &color);

private:
    QColor dropShadowColor;
    QColor textColor;
    void drawText(QPainter *painter, QPoint offset);
    void drawTextEffect(QPainter *painter, QPoint offset);
};

class YACReaderTitledToolBar : public QWidget
{
    Q_OBJECT
public:
    explicit YACReaderTitledToolBar(const QString &title, QWidget *parent = 0);

signals:
    void cancelOperationRequested();

public slots:
    void addAction(QAction *action);
    void addSpacing(int space);
    void addSepartor();
    void showBusyIndicator();
    void hideBusyIndicator();

private:
    DropShadowLabel *nameLabel;
    BusyIndicator *busyIndicator;
};

#endif // YACREADER_TITLED_TOOLBAR_H
