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
    void open();

protected:
    QLineEdit * edit;
    QListWidget * list;

    QPushButton * acceptButton;
    QPushButton * cancelButton;
};

#endif // ADD_LABEL_DIALOG_H
