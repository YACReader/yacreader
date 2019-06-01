#ifndef EMPTY_CONTAINER_INFO_H
#define EMPTY_CONTAINER_INFO_H

#include <QtWidgets>

class EmptyContainerInfo : public QWidget
{
    Q_OBJECT
public:
    explicit EmptyContainerInfo(QWidget *parent = 0);
    void setPixmap(const QPixmap &pixmap);
    void setText(const QString &text);
    QVBoxLayout *setUpDefaultLayout(bool addStretch);
signals:

public slots:

protected:
    void paintEvent(QPaintEvent *);

    QLabel *iconLabel;
    QLabel *titleLabel;
    QString backgroundColor;
};

#endif // EMPTY_CONTAINER_INFO_H
