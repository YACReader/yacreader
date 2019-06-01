#ifndef NOTIFICATIONS_LABEL_WIDGET_H
#define NOTIFICATIONS_LABEL_WIDGET_H

#include <QWidget>

class QLabel;
class QPropertyAnimation;
class QGraphicsOpacityEffect;

class NotificationsLabelWidget : public QWidget
{
    Q_OBJECT
private:
    QLabel *textLabel;
    QPropertyAnimation *anim;
    QGraphicsOpacityEffect *effect;

protected:
    void paintEvent(QPaintEvent *);

public:
    NotificationsLabelWidget(QWidget *parent);

public slots:
    void flash();
    void setText(const QString &text);
    void updatePosition();
};

#endif
