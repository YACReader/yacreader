#ifndef YACREADER_TITLED_TOOLBAR_H
#define YACREADER_TITLED_TOOLBAR_H

#include <QWidget>

class QIcon;

class YACReaderTitledToolBar : public QWidget
{
    Q_OBJECT
public:
    explicit YACReaderTitledToolBar(const QString & title, QWidget *parent = 0);
    
signals:
    
public slots:
    void addAction(QAction * action);
};

#endif // YACREADER_TITLED_TOOLBAR_H
