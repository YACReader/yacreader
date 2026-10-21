#ifndef SMART_LIST_DIALOG_H
#define SMART_LIST_DIALOG_H

#include <QDialog>

class QLineEdit;
class QPushButton;
class QSettings;

// the name and the search of a smart list; while it is open, the library shows what the search finds. Standard
// widgets only: the dialog is not themed
class SmartListDialog : public QDialog
{
    Q_OBJECT

public:
    // an empty name opens "New smart list", else "Edit smart list"
    SmartListDialog(QSettings *settings, const QString &name, const QString &query, QWidget *parent = nullptr);

    QString name() const;
    QString query() const;

signals:
    void searchChanged(const QString &query);

private:
    void updateSaveButton();

    QLineEdit *nameEdit;
    QLineEdit *searchEdit;
    QPushButton *saveButton;
};

#endif
