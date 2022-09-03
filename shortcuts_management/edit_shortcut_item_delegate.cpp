#include "edit_shortcut_item_delegate.h"

#include <QAction>

EditShortcutItemDelegate::EditShortcutItemDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

QWidget *EditShortcutItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    KeySequenceLineEdit *editor = new KeySequenceLineEdit(parent);
    connect(editor, &QLineEdit::editingFinished, this, &EditShortcutItemDelegate::closeShortcutEditor);
    return editor;
}

void EditShortcutItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::DisplayRole).toString();

    KeySequenceLineEdit *lineEdit = static_cast<KeySequenceLineEdit *>(editor);
    lineEdit->setText(value);
}

void EditShortcutItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    KeySequenceLineEdit *lineEdit = static_cast<KeySequenceLineEdit *>(editor);

    model->setData(index, lineEdit->text(), Qt::EditRole);
}

void EditShortcutItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &mi) const
{
    Q_UNUSED(mi);

    editor->setGeometry(option.rect);
}

bool EditShortcutItemDelegate::eventFilter(QObject *editor, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
        return false;
    return QItemDelegate::eventFilter(editor, event);
}

void EditShortcutItemDelegate::closeShortcutEditor()
{
    emit commitData(static_cast<QWidget *>(sender()));
    emit closeEditor(static_cast<QWidget *>(sender()), QAbstractItemDelegate::NoHint);
}

// TODO uncoment commented code for enabling concatenated shortcuts
KeySequenceLineEdit::KeySequenceLineEdit(QWidget *parent)
    : QLineEdit(parent) //,numKeys(0)
{
    // keys[0] = keys[1] = keys[2] = keys[3] = 0;
    setAlignment(Qt::AlignRight);

    QPixmap clearPixmap(":/images/clear_shortcut.png");
    QPixmap acceptPixmap(":/images/accept_shortcut.png");

    clearButton = new QToolButton(this);
    acceptButton = new QToolButton(this);
    QString buttonsStyle = "QToolButton { border: none; padding: 0px; }";

    clearButton->setIcon(QIcon(clearPixmap));
    clearButton->setIconSize(clearPixmap.size());
    clearButton->setCursor(Qt::ArrowCursor);
    clearButton->setStyleSheet(buttonsStyle);

    acceptButton->setIcon(QIcon(acceptPixmap));
    acceptButton->setIconSize(acceptPixmap.size());
    acceptButton->setCursor(Qt::ArrowCursor);
    acceptButton->setStyleSheet(buttonsStyle);

    connect(clearButton, &QAbstractButton::clicked, this, &QLineEdit::clear);
    connect(acceptButton, &QAbstractButton::clicked, this, &QLineEdit::editingFinished);
}

void KeySequenceLineEdit::resizeEvent(QResizeEvent *e)
{
    QSize szClear = clearButton->sizeHint();
    // int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    int leftMargin = style()->pixelMetric(QStyle::PM_LayoutLeftMargin);
    // int topMargin = style()->pixelMetric(QStyle::PM_LayoutTopMargin);
    clearButton->move(0 + leftMargin, (e->size().height() - 19) / 2); // 16 is the icon height+1blank pixel

    acceptButton->move(leftMargin + szClear.width(), (e->size().height() - 19) / 2);
}

void KeySequenceLineEdit::keyPressEvent(QKeyEvent *e)
{
    int key = e->key();

    // if ( numKeys > 3 ||
    if (key == Qt::Key_Control ||
        key == Qt::Key_Shift ||
        key == Qt::Key_Meta ||
        key == Qt::Key_Alt)
        return;

    key |= translateModifiers(e->modifiers(), e->text());

    /*switch (numKeys) {
        case 0:
            keys[0] = nextKey;
            break;
        case 1:
            keys[1] = nextKey;
            break;
        case 2:
            keys[2] = nextKey;
            break;
        case 3:
            keys[3] = nextKey;
            break;
        default:
            break;
    }*/
    // numKeys++;
    QKeySequence keySequence = QKeySequence(key);
    setText(keySequence.toString(QKeySequence::NativeText));
    e->accept();
}

int KeySequenceLineEdit::translateModifiers(Qt::KeyboardModifiers state,
                                            const QString &text)
{
    int result = 0;
    // The shift modifier only counts when it is not used to type a symbol
    // that is only reachable using the shift key anyway
    if ((state & Qt::ShiftModifier) && (text.size() == 0 || !text.at(0).isPrint() || text.at(0).isLetterOrNumber() || text.at(0).isSpace()))
        result |= Qt::SHIFT;
    if (state & Qt::ControlModifier)
        result |= Qt::CTRL;
    if (state & Qt::MetaModifier)
        result |= Qt::META;
    if (state & Qt::AltModifier)
        result |= Qt::ALT;
    return result;
}
