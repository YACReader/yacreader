#ifndef ADD_LABEL_DIALOG_H
#define ADD_LABEL_DIALOG_H

#include <QtWidgets>

class AddLabelDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddLabelDialog(QWidget *parent = 0);

signals:

public slots:

protected:
    QLineEdit * edit;
    QListWidget * list;

};

#endif // ADD_LABEL_DIALOG_H
