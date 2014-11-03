#ifndef YACREADER_MACOSX_TOOLBAR_H
#define YACREADER_MACOSX_TOOLBAR_H

#include <QMacToolBar>
#include <QtWidgets>

class YACReaderMacOSXToolbar : public QMacToolBar
{
    Q_OBJECT
public:
    explicit YACReaderMacOSXToolbar(QObject *parent = 0);
    void addAction(QAction * action);
    void addDropDownItem(const QList<QAction*> & actions, const QAction * defaultAction = 0);
    void addSpace(int size); //size in points
    void addSeparator();
    void addStretch();
    void addWidget(QWidget * widget);
    void show();
    void hide();

signals:

public slots:

protected:
    NSToolbar * nativeToolBar;

};

#endif // YACREADER_MACOSX_TOOLBAR_H
