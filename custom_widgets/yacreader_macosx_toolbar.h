#ifndef YACREADER_MACOSX_TOOLBAR_H
#define YACREADER_MACOSX_TOOLBAR_H

#include <QMacToolBar>
#include <QtWidgets>

#include "yacreader_global.h"

// Wrapper for NSTextField
class YACReaderMacOSXSearchLineEdit : public QObject
{
    Q_OBJECT
public:
    YACReaderMacOSXSearchLineEdit();
    void setFocus(Qt::FocusReason reason);
    void *getNSTextField();

public slots:
    QString text();
    void clear();
    void clearText(); // no signal emited
    void setDisabled(bool disabled);
    void setEnabled(bool enabled);

private:
    void *nstextfield;

signals:
    // convenience signal for YACReaderLibrary search edit
    void filterChanged(QString);
};

class MacToolBarItemWrapper : public QObject
{
    Q_OBJECT
public:
    MacToolBarItemWrapper(QAction *action, QMacToolBarItem *toolbaritem);

public slots:
    void actionToggled(bool toogled);

private:
    QAction *action;
    QMacToolBarItem *toolbaritem;

    void updateIcon(bool checked);
};

class YACReaderMacOSXToolbar : public QMacToolBar
{
    Q_OBJECT
public:
    explicit YACReaderMacOSXToolbar(QObject *parent = 0);
    void addAction(QAction *action);
    void addDropDownItem(const QList<QAction *> &actions, const QAction *defaultAction = 0);
    void addSpace(int size); // size in points
    void addSeparator();
    void addStretch();
    void addWidget(QWidget *widget);
    void show();
    void hide();
    QMap<QString, QAction *> actions;

    // hacks everywhere
    // convenience method for YACReaderLibrary search edit
    YACReaderMacOSXSearchLineEdit *addSearchEdit();
    // convenience method for showing the fit to width slider in MacOSX
    QAction *addFitToWidthSlider(QAction *attachToAction);

    // convenience method for switching the icon of the view selector
    void updateViewSelectorIcon(const QIcon &icon);

signals:

public slots:

protected:
    NSToolbar *nativeToolBar;
    void *delegate;
    bool yosemite;
    QMacToolBarItem *viewSelector;
};

#endif // YACREADER_MACOSX_TOOLBAR_H
