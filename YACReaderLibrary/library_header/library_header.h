#ifndef LIBRARY_HEADER_H
#define LIBRARY_HEADER_H

#include "themable.h"

#include <QObject>
#include <QWidget>

class HeaderModel;
class QLabel;
class QStyle;
class QToolButton;
class ViewSelector;
class YACReaderSearchLineEdit;

// Flat header buttons: no 1 px content shift when pressed or checked.
QStyle *headerButtonStyle();

struct HeaderColors {
    QColor sidebarBackground;
    QColor sidebarText;
    QColor sidebarButtonHover;
    QColor sidebarButtonPressed;
    QColor toolbarBackground;
    QColor folderName;
    QColor icon;
    QColor buttonHover;
    QColor buttonPressed;
    QColor selectorBackground;
    QColor selectorHover;
    QColor selectorPressed;
    QColor selectorChecked;
};

HeaderColors headerColors(const Theme &theme);

// Top of the sidebar column: app identity, settings, server, help.
class SidebarHeader : public QWidget
{
    Q_OBJECT
public:
    explicit SidebarHeader(HeaderModel *model, QWidget *parent = nullptr);
    void applyTheme(const HeaderColors &colors, const QIcon &appIcon);
    // App icon and name; hidden when the system title bar already shows them.
    void setAppIdentityVisible(bool visible);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QColor background;
    QLabel *appIcon = nullptr;
    QLabel *appName = nullptr;
    QList<QToolButton *> buttons;
    void relayout();
};

// Top of the content column: navigation, folder name, search, view selector
// and full screen.
class ContentHeader : public QWidget
{
    Q_OBJECT
public:
    explicit ContentHeader(HeaderModel *model, QWidget *parent = nullptr);
    void applyTheme(const HeaderColors &colors);
    YACReaderSearchLineEdit *searchField() const { return search; }
    QLabel *folderNameLabel() const { return folderName; }

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QColor background;
    QToolButton *back;
    QToolButton *forward;
    QToolButton *fullscreen = nullptr;
    QLabel *folderName;
    QString folderNameText;
    YACReaderSearchLineEdit *search;
    bool settingSearchText = false;
    ViewSelector *selector;
    void relayout();
};

// Replaces YACReaderMainToolBar: the Qt widget view of the header model, in
// two parts that LibraryWindow places at the top of the splitter columns.
class LibraryHeader : public QObject, protected Themable
{
    Q_OBJECT
public:
    // Header icons are drawn on an 18 unit grid; 18 maps it 1:1 to pixels at 100%.
    static constexpr int IconSize = 18;
    static constexpr int ButtonSize = IconSize + 10;
    static constexpr int Height = 40;

    explicit LibraryHeader(HeaderModel *model, QObject *parent = nullptr);

    SidebarHeader *sidebarPart() const { return sidebar; }
    ContentHeader *contentPart() const { return content; }

    void setVisible(bool visible);
    void setAppIdentityVisible(bool visible);

protected:
    void applyTheme(const Theme &theme) override;

private:
    SidebarHeader *sidebar;
    ContentHeader *content;
};

#endif // LIBRARY_HEADER_H
