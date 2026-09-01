#ifndef YACREADER_SEARCH_LINE_EDIT_H
#define YACREADER_SEARCH_LINE_EDIT_H

#include "themable.h"

#include <QCompleter>
#include <QLineEdit>

class QToolButton;
class QLabel;
class QMenu;

class YACReaderSearchLineEdit : public QLineEdit, protected Themable
{
    Q_OBJECT

public:
    YACReaderSearchLineEdit(QWidget *parent = 0);
    void clearText(); // no signal emited;
    const QString text();
    void setSearchMenu(QMenu *menu);

protected:
    void resizeEvent(QResizeEvent *) override;
    void applyTheme(const Theme &theme) override;

signals:
    void filterChanged(QString);

private slots:
    void updateCloseButton(const QString &text);
    void processText(const QString &text);

private:
    QToolButton *clearButton;
    QToolButton *menuButton;
    QLabel *searchLabel;

    int paddingLeft;
    int paddingRight;
};

#endif // YACREADER_SEARCH_LINE_EDIT_H
