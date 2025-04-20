#ifndef YACREADER_MACOSX_TOOLBAR_H
#define YACREADER_MACOSX_TOOLBAR_H

#include <QtGlobal>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QMacToolBar>
#include <QtWidgets>
#include <QMainWindow>

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

    void attachToWindow(QMainWindow *window);

signals:

public slots:

protected:
    NSToolbar *nativeToolBar;
    void *delegate;
    bool yosemite;
    QMacToolBarItem *viewSelector;
};
#else

#ifdef YACREADER_LIBRARY

#include "yacreader_main_toolbar.h"
#include "yacreader_search_line_edit.h"
#include <QMainWindow>

class YACReaderMacOSXSearchLineEdit : public YACReaderSearchLineEdit
{
};

class YACReaderMacOSXToolbar : public YACReaderMainToolBar
{
    Q_OBJECT
public:
    explicit YACReaderMacOSXToolbar(QWidget *parent = 0);
    QSize sizeHint() const override;
    void addAction(QAction *action);
    void addSpace(int size); // size in points
    void addStretch();
    YACReaderMacOSXSearchLineEdit *addSearchEdit();
    void updateViewSelectorIcon(const QIcon &icon);
    void attachToWindow(QMainWindow *window);

    void *getSearchEditDelegate() { return searchEditDelegate; };

    void emitFilterChange(const QString &filter) { emit filterChanged(filter); };

    QAction *actionFromIdentifier(const QString &identifier);
signals:
    void filterChanged(QString);

private:
    void paintEvent(QPaintEvent *) override;

    void *searchEditDelegate;
};

#else

#include <QtWidgets>

class YACReaderMacOSXToolbar : public QWidget
{
    Q_OBJECT
public:
    explicit YACReaderMacOSXToolbar(QWidget *parent = 0);
    void attachToWindow(QMainWindow *window);
    void addStretch();

    void setMovable(bool movable) {};
    void addSeparator() {};

    void setIconSize(const QSize &size) {};

public slots:
    void setHidden(bool hidden);
    void show();
    void hide();
};

#endif

#endif

#endif // YACREADER_MACOSX_TOOLBAR_H
