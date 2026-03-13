#ifndef EMPTY_CONTAINER_INFO_H
#define EMPTY_CONTAINER_INFO_H

#include <QLabel>
#include <QPaintEvent>
#include <QPixmap>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include "themable.h"

class EmptyContainerInfo : public QWidget, protected Themable
{
    Q_OBJECT
public:
    explicit EmptyContainerInfo(QWidget *parent = nullptr);
    void setPixmap(const QPixmap &pixmap);
    void setText(const QString &text);
    QVBoxLayout *setUpDefaultLayout(bool addStretch);
signals:

public slots:

protected:
    void paintEvent(QPaintEvent *) override;
    void applyTheme(const Theme &theme) override;

    QLabel *iconLabel;
    QLabel *titleLabel;
};

#endif // EMPTY_CONTAINER_INFO_H
