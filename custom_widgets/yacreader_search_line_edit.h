#ifndef YACREADER_SEARCH_LINE_EDIT_H
#define YACREADER_SEARCH_LINE_EDIT_H

#include <QLineEdit>

class QToolButton;
class QLabel;

class YACReaderSearchLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    YACReaderSearchLineEdit(QWidget *parent = 0);

protected:
    void resizeEvent(QResizeEvent *);

private slots:
    void updateCloseButton(const QString &text);

private:
    QToolButton *clearButton;
	QLabel * searchLabel;
};



#endif // YACREADER_SEARCH_LINE_EDIT_H
