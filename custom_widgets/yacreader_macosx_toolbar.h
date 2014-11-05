#ifndef YACREADER_MACOSX_TOOLBAR_H
#define YACREADER_MACOSX_TOOLBAR_H

#include <QMacToolBar>
#include <QtWidgets>

#include "yacreader_global.h"

//Wrapper for NSTextField
class YACReaderMacOSXSearchLineEdit : public QObject
{
    Q_OBJECT
public:
    YACReaderMacOSXSearchLineEdit();
    void * getNSTextField();

public slots:
    QString text();
    void clear();
    void setDisabled(bool disabled);
    void setEnabled(bool enabled);

private:
    void * nstextfield;


signals:
    //convenience signal for YACReaderLibrary search edit
    void filterChanged(YACReader::SearchModifiers, QString);
};

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
    QMap<QString, QAction *> actions;

    //convenience method for YACReaderLibrary search edit
    YACReaderMacOSXSearchLineEdit *addSearchEdit();

signals:

public slots:

protected:
    NSToolbar * nativeToolBar;
    void *delegate;


};

#endif // YACREADER_MACOSX_TOOLBAR_H
