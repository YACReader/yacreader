#ifndef YACREADER_MACOSX_TOOLBAR_H
#define YACREADER_MACOSX_TOOLBAR_H

#include <QMacToolBar>

class YACReaderMacOSXToolbar : public QMacToolBar
{
    Q_OBJECT
public:
    explicit YACReaderMacOSXToolbar(QWidget *window, QObject *parent = 0);

signals:

public slots:

};

#endif // YACREADER_MACOSX_TOOLBAR_H
