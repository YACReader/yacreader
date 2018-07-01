#include "yacreader_field_plain_text_edit.h"

#include <QAction>

YACReaderFieldPlainTextEdit::YACReaderFieldPlainTextEdit(QWidget * parent)
	:QPlainTextEdit(parent)
{
	document()->setModified(false);
	setPlainText(tr("Click to overwrite"));
	restore = new QAction(tr("Restore to default"),this);
	this->addAction(restore);
	//this->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void YACReaderFieldPlainTextEdit::focusInEvent(QFocusEvent* e)
{
	if (e->reason() == Qt::MouseFocusReason  || e->reason() == Qt::TabFocusReason)
	{
	  document()->setModified(true);
	  if(toPlainText()==tr("Click to overwrite"))
		setPlainText("");
	}

	QPlainTextEdit::focusInEvent(e);
}

void YACReaderFieldPlainTextEdit::focusOutEvent(QFocusEvent* e)
{
	/*if (e->reason() == Qt::MouseFocusReason  || e->reason() == Qt::TabFocusReason)
	{
		if(toPlainText().isEmpty())
		{
			setPlainText(tr("Click to overwrite"));
			document()->setModified(false);
		}
	}
	*/
	QPlainTextEdit::focusOutEvent(e);
}

void YACReaderFieldPlainTextEdit::clear()
{
	QPlainTextEdit::clear();
	document()->setModified(false);
	setPlainText(tr("Click to overwrite"));
}

void YACReaderFieldPlainTextEdit::setDisabled(bool disabled)
{
	if(disabled)
		setPlainText(tr("Click to overwrite"));
	QPlainTextEdit::setDisabled(disabled);
}
