#ifndef ADD_LABEL_DIALOG_H
#define ADD_LABEL_DIALOG_H

#include <QtWidgets>

#include "yacreader_global.h"

class AddLabelDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddLabelDialog(QWidget *parent = 0);
    YACReader::LabelColors selectedColor();
    QString name();
signals:

public slots:
    int exec();

protected slots:
    void validateName(const QString & name);

protected:
    QLineEdit * edit;
    QListWidget * list;

    QPushButton * acceptButton;
    QPushButton * cancelButton;
};

#endif // ADD_LABEL_DIALOG_H
