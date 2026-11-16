#ifndef HEADER_MODEL_H
#define HEADER_MODEL_H

#include <QObject>
#include <QString>

class QAction;
class QMenu;

struct HeaderActions {
    QAction *back = nullptr;
    QAction *forward = nullptr;
    QAction *settings = nullptr;
    QAction *server = nullptr;
    QAction *help = nullptr;
    // nullptr on macOS, where the green traffic light does this.
    QAction *fullscreen = nullptr;
    QAction *gridView = nullptr;
    QAction *flowView = nullptr;
    QAction *infoView = nullptr;
};

// What the header shows, without widgets. LibraryWindow sets the state here;
// the header views show it and report user input back.
class HeaderModel : public QObject
{
    Q_OBJECT
public:
    explicit HeaderModel(const HeaderActions &actions, QObject *parent = nullptr);

    const HeaderActions &actions() const { return headerActions; }

    QString folderName() const { return currentFolderName; }
    void setFolderName(const QString &name);

    bool isEnabled() const { return enabled; }
    void setEnabled(bool enabled);

    QString searchText() const { return currentSearchText; }
    // notify: run the search, as if the user typed the text. Nothing happens
    // when the text does not change.
    void setSearchText(const QString &text, bool notify = true);
    bool isSearchEnabled() const { return searchEnabled; }
    void setSearchEnabled(bool enabled);
    QMenu *searchMenu() const { return menu; }
    void setSearchMenu(QMenu *menu);
    void focusSearch();

    // For views: the user changed the search text.
    void editSearch(const QString &text);

signals:
    void folderNameChanged(const QString &name);
    void enabledChanged(bool enabled);
    // Views show the text without reporting it back.
    void searchTextSet(const QString &text);
    void searchEnabledChanged(bool enabled);
    void searchMenuChanged(QMenu *menu);
    void searchFocusRequested();
    // The search to run.
    void searchChanged(const QString &text);

private:
    HeaderActions headerActions;
    QString currentFolderName;
    QString currentSearchText;
    QMenu *menu = nullptr;
    bool enabled = true;
    bool searchEnabled = true;
};

#endif // HEADER_MODEL_H
