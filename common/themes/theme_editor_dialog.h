#ifndef THEME_EDITOR_DIALOG_H
#define THEME_EDITOR_DIALOG_H

#include <QDialog>
#include <QJsonObject>

class QComboBox;
class QLabel;
class QLineEdit;
class QTreeWidget;
class QTreeWidgetItem;

// Generic theme parameter editor.
// Works entirely on QJsonObject — has no knowledge of app-specific ThemeParams.
// Connect to themeJsonChanged to receive live updates as the user edits colors.
class ThemeEditorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ThemeEditorDialog(const QJsonObject &params, QWidget *parent = nullptr);

    QJsonObject currentParams() const { return params; }

    void updateSavedId(const QString &id);

signals:
    void themeJsonChanged(const QJsonObject &params);
    void saveToLibraryRequested(const QJsonObject &json);

private:
    void populate(QTreeWidgetItem *parent, const QJsonObject &obj, const QStringList &path);
    void editColorItem(QTreeWidgetItem *item);
    void applyColorToItem(QTreeWidgetItem *item, const QColor &color);
    void toggleBoolItem(QTreeWidgetItem *item);
    void editNumberItem(QTreeWidgetItem *item);
    void filterTree(const QString &query);
    void saveToFile();
    void loadFromFile();

    // Identify feature: hold the (i) button to temporarily flash the UI element
    // that uses the current item's value. The original value is restored on release.
    void identifyPressed();
    void identifyReleased();

    static QIcon colorIcon(const QColor &color);
    static void setJsonPath(QJsonObject &root, const QStringList &path, const QJsonValue &value);

    QTreeWidget *tree;
    QLineEdit *searchEdit;
    QJsonObject params;

    // Meta UI
    QLabel *idLabel;
    QLineEdit *nameEdit;
    QComboBox *variantCombo;

    void syncMetaToParams();
    void syncMetaFromParams();

    // Identify state (null item = inactive)
    QTreeWidgetItem *identifyItem = nullptr;
    QStringList identifyPath;
    QJsonValue identifySnapshot; // original value saved on press, restored on release
};

#endif // THEME_EDITOR_DIALOG_H
