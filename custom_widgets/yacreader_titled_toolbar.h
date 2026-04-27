#ifndef YACREADER_TITLED_TOOLBAR_H
#define YACREADER_TITLED_TOOLBAR_H

#include "themable.h"

#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QPoint>
#include <QWidget>

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

class YACReaderTitledToolBar : public QWidget, protected Themable
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
    void setTitle(const QString &title);

protected:
    void applyTheme(const Theme &theme) override;

private:
    DropShadowLabel *nameLabel;
    BusyIndicator *busyIndicator;
    QList<QWidget *> separators;
};

#endif // YACREADER_TITLED_TOOLBAR_H
