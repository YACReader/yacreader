#ifndef YACREADER_SEARCH_LINE_EDIT_H
#define YACREADER_SEARCH_LINE_EDIT_H

#include <QLineEdit>
#include <QCompleter>

#include "yacreader_global.h"

class QToolButton;
class QLabel;

class YACReaderSearchLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    YACReaderSearchLineEdit(QWidget *parent = 0);
    void clearText(); //no signal emited;
    const QString text();

protected:
    void resizeEvent(QResizeEvent *);

signals:
    void filterChanged(const YACReader::SearchModifiers, QString);

private slots:
    void updateCloseButton(const QString &text);
    void processText(const QString &text);

private:
    QToolButton *clearButton;
    QLabel *searchLabel;
    QCompleter *modifiersCompleter;
    QStringList modifiers;
};

#endif // YACREADER_SEARCH_LINE_EDIT_H
