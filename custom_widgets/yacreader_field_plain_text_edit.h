#ifndef YACREADER_FIELD_PLAIN_TEXT_EDIT_H
#define YACREADER_FIELD_PLAIN_TEXT_EDIT_H

#include <QPlainTextEdit>

class QAction;
class QFocusEvent;


class YACReaderFieldPlainTextEdit : public QPlainTextEdit
{
  Q_OBJECT
	public:
		YACReaderFieldPlainTextEdit(QWidget * parent = 0);
		void clear();
		void setDisabled(bool disabled);
	protected:
		void focusInEvent(QFocusEvent* e);
		void focusOutEvent(QFocusEvent* e);
private:
	QAction * restore;

};

#endif // YACREADER_FIELD_PLAIN_TEXT_EDIT_H