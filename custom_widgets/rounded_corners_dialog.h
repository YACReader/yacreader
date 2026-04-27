#ifndef ROUNDEDCORNERSDIALOG_H
#define ROUNDEDCORNERSDIALOG_H

#include <QColor>
#include <QDialog>

namespace YACReader {
class RoundedCornersDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RoundedCornersDialog(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;
    void setBackgroundColor(const QColor &color);

private:
    QColor m_backgroundColor { 255, 255, 255 };
};
}

#endif // ROUNDEDCORNERSDIALOG_H
