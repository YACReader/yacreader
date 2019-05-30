#include "yacreader_field_edit.h"

#include <QAction>
#include <QFocusEvent>

YACReaderFieldEdit::YACReaderFieldEdit(QWidget *parent)
    : QLineEdit(parent)
{
    setPlaceholderText(tr("Click to overwrite"));
    setModified(false);
    restore = new QAction(tr("Restore to default"), this);
    this->addAction(restore);
    //this->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void YACReaderFieldEdit::focusInEvent(QFocusEvent *e)
{
    if (e->reason() == Qt::MouseFocusReason) {
        setModified(true);
        setPlaceholderText("");
    }

    QLineEdit::focusInEvent(e);
}

void YACReaderFieldEdit::clear()
{
    setPlaceholderText(tr("Click to overwrite"));
    QLineEdit::clear();
    QLineEdit::setModified(false);
}

void YACReaderFieldEdit::setDisabled(bool disabled)
{
    if (disabled)
        setPlaceholderText("");
    QLineEdit::setDisabled(disabled);
}