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
    // Space for the macOS traffic lights.
    void setLeadingInset(int inset);
    void setControlsVisible(bool visible);
    QList<QWidget *> interactiveWidgets() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QColor background;
    QLabel *appIcon = nullptr;
    QLabel *appName = nullptr;
    QList<QToolButton *> buttons;
    int leadingInset = 0;
    bool identityVisible = true;
    bool controlsVisible = true;
    void updateVisibility();
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
    // Space for the Windows caption buttons.
    void setTrailingInset(int inset);
    void setControlsVisible(bool visible);
    QList<QWidget *> interactiveWidgets() const;

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
    int trailingInset = 0;
    void relayout();
};

// Replaces YACReaderMainToolBar: the Qt widget view of the header model, in
// two parts that LibraryWindow places at the top of the splitter columns. The
// window frame asks it which pixels are interactive and which drag the window.
class LibraryHeader : public QObject, protected Themable
{
    Q_OBJECT
public:
    // Header icons are drawn on an 18 unit grid; 18 maps it 1:1 to pixels at 100%.
    static constexpr int IconSize = 18;
    static constexpr int ButtonSize = IconSize + 10;
    static constexpr int Height = 40;

    enum class Hit { Outside,
                     Client,
                     Caption };

    explicit LibraryHeader(HeaderModel *model, QObject *parent = nullptr);

    SidebarHeader *sidebarPart() const { return sidebar; }
    ContentHeader *contentPart() const { return content; }

    void setVisible(bool visible);
    void setAppIdentityVisible(bool visible);
    void setHeight(int height);
    int height() const { return headerHeight; }
    void setLeadingInset(int inset);
    void setTrailingInset(int inset);
    // false: the parts only paint their background (native macOS items on top).
    void setControlsVisible(bool visible);

    // windowPos: logical coordinates relative to the top-level window.
    Hit hitTest(const QPoint &windowPos) const;
    // Right edge of the sidebar part, in top-level window coordinates.
    int sidebarRightEdge() const;

protected:
    void applyTheme(const Theme &theme) override;

private:
    SidebarHeader *sidebar;
    ContentHeader *content;
    int headerHeight = Height;
};

#endif // LIBRARY_HEADER_H
