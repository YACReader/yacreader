#ifndef ROUNDEDCORNERSDIALOG_H
#define ROUNDEDCORNERSDIALOG_H

#include <QDialog>

namespace YACReader {
class RoundedCornersDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RoundedCornersDialog(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;
};
}

#endif // ROUNDEDCORNERSDIALOG_H
