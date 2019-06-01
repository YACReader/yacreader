#ifndef YACREADER_TITLED_TOOLBAR_H
#define YACREADER_TITLED_TOOLBAR_H

#include "theme.h"

#include <QWidget>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QPoint>

class QIcon;

class DropShadowLabel : public QLabel
{
    Q_OBJECT

public:
    DropShadowLabel(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event);
    void setColor(const QColor &color);
    void setDropShadowColor(const QColor &color);

private:
    QColor dropShadowColor;
    QColor textColor;
    void drawText(QPainter *painter, QPoint offset);
    void drawTextEffect(QPainter *painter, QPoint offset);

    Theme theme = Theme::currentTheme();
};

class YACReaderTitledToolBar : public QWidget
{
    Q_OBJECT
public:
    explicit YACReaderTitledToolBar(const QString &title, QWidget *parent = nullptr);

signals:

public slots:
    void addAction(QAction *action);
    void addSpacing(int space);
    void addSepartor();

private:
    DropShadowLabel *nameLabel;
    Theme theme = Theme::currentTheme();
};

#endif // YACREADER_TITLED_TOOLBAR_H
