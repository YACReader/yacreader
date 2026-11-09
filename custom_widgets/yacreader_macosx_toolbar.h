#ifndef YACREADER_MACOSX_TOOLBAR_H
#define YACREADER_MACOSX_TOOLBAR_H

#include <QtWidgets>

class YACReaderMacOSXToolbar : public QWidget
{
    Q_OBJECT
public:
    explicit YACReaderMacOSXToolbar(QWidget *parent = 0);
    void attachToWindow(QMainWindow *window);
    void addStretch();

    void setMovable(bool movable) { };
    void addSeparator();

    void setIconSize(const QSize &size) { };

public slots:
    void setHidden(bool hidden);
    void show();
    void hide();
};

#endif // YACREADER_MACOSX_TOOLBAR_H
