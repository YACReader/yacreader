#ifndef ADD_LABEL_DIALOG_H
#define ADD_LABEL_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>

#include "yacreader_global.h"
#include "themable.h"

class AddLabelDialog : public QDialog, protected Themable
{
    Q_OBJECT
public:
    explicit AddLabelDialog(QWidget *parent = nullptr);
    YACReader::LabelColors selectedColor();
    QString name();
signals:

public slots:
    int exec() override;

protected slots:
    void validateName(const QString &name);

protected:
    void applyTheme(const Theme &theme) override;

    QLineEdit *edit;
    QListWidget *list;

    QPushButton *acceptButton;
    QPushButton *cancelButton;
};

#endif // ADD_LABEL_DIALOG_H
