#ifndef YACREADER_LIBRARY_ITEM_WIDGET_H
#define YACREADER_LIBRARY_ITEM_WIDGET_H

#include "themable.h"

#include <QPixmap>
#include <QWidget>

class QLabel;
class QToolButton;
class QMouseEvent;
class QEvent;

class YACReaderLibraryItemWidget : public QWidget, protected Themable
{
    Q_OBJECT

public:
    YACReaderLibraryItemWidget(QString name, QString path, QWidget *parent = 0);
    QString name;
    QString path;

signals:
    void selected(QString, QString);
    void showOptions();

public slots:
    void showUpDownButtons(bool show);

    // bool eventFilter(QObject *object, QEvent *event);
    void select();
    void deselect();
    void setName(const QString &name);

private:
    QLabel *icon;
    QLabel *nameLabel;
    QLabel *optionsIconLabel;

    QToolButton *options;
    QToolButton *up;
    QToolButton *down;

    bool isSelected;

protected:
    void applyTheme(const Theme &theme) override;
};

#endif // YACREADER_LIBRARY_ITEM_WIDGET_H
