#ifndef PAGE_LABEL_WIDGET_H
#define PAGE_LABEL_WIDGET_H

#include <QWidget>

#include "themable.h"

class QLabel;
class QPropertyAnimation;

class PageLabelWidget : public QWidget, protected Themable
{
    Q_OBJECT
private:
    QLabel *textLabel;
    QPropertyAnimation *animation;
    QColor infoBackgroundColor;
    int fontSizePx = 0;

protected:
    void paintEvent(QPaintEvent *) override;
    void applyTheme() override;

public:
    PageLabelWidget(QWidget *parent);

public slots:
    void show();
    void hide();
    void setText(const QString &text);
    void updatePosition();
};

#endif
