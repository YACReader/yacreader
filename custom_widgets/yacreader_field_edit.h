#ifndef YACREADER_FIELD_EDIT_H
#define YACREADER_FIELD_EDIT_H

#include <QLineEdit>

class QAction;
class QFocusEvent;

class YACReaderFieldEdit : public QLineEdit
{
  Q_OBJECT
	public:
		YACReaderFieldEdit(QWidget * parent = 0);
		void clear();
		void setDisabled(bool disabled);
	protected:
		void focusInEvent(QFocusEvent* e);
private:
	QAction * restore;

};

#endif // YACREADER_FIELD_EDIT_H