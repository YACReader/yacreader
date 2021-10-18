#ifndef EDIT_SHORTCUT_ITEM_DELEGATE_H
#define EDIT_SHORTCUT_ITEM_DELEGATE_H

#include <QItemDelegate>
#include <QLineEdit>
#include <QKeyEvent>
#include <QKeySequence>
#include <QToolButton>

class KeySequenceLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit KeySequenceLineEdit(QWidget *parent = 0);

protected:
    // int numKeys;
    // int keys[4];
    void keyPressEvent(QKeyEvent *);
    int translateModifiers(Qt::KeyboardModifiers state, const QString &text);
    void resizeEvent(QResizeEvent *);

private:
    QToolButton *clearButton;
    QToolButton *acceptButton;
};

class EditShortcutItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit EditShortcutItemDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &mi) const;
    bool eventFilter(QObject *editor, QEvent *event);
signals:

public slots:
    void closeShortcutEditor();
};

#endif // EDIT_SHORTCUT_ITEM_DELEGATE_H
