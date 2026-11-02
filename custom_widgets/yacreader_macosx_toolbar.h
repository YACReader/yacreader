#ifndef YACREADER_MACOSX_TOOLBAR_H
#define YACREADER_MACOSX_TOOLBAR_H

#ifdef YACREADER_LIBRARY

#include "yacreader_main_toolbar.h"
#include "yacreader_search_line_edit.h"

#include <QMainWindow>

class QMenu;

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

    void setNativeSearchField(void *field);
    void setSearchMenu(QMenu *menu);
    void setSearchText(const QString &text, bool notify = true);
    void clearSearchText(bool notify = true);
    void focusSearch();
    void setSearchEnabled(bool enabled);
    QString searchText() const;
    void nativeSearchTextChanged(const QString &text);

    QAction *actionFromIdentifier(const QString &identifier);
signals:
    void filterChanged(QString);

private:
    void paintEvent(QPaintEvent *) override;

    void *searchEditDelegate;
    void *nativeSearchField;
    QMenu *searchMenu;
    YACReaderMacOSXSearchLineEdit *searchEditProxy;
    QString pendingSearchText;
    bool searchEnabled;
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

    void setMovable(bool movable) { };
    void addSeparator();

    void setIconSize(const QSize &size) { };

public slots:
    void setHidden(bool hidden);
    void show();
    void hide();
};

#endif

#endif // YACREADER_MACOSX_TOOLBAR_H
