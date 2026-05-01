#ifndef WHATSNEWDIALOG_H
#define WHATSNEWDIALOG_H

#include "rounded_corners_dialog.h"
#include "themable.h"

class QLabel;
class QPushButton;

namespace YACReader {

class WhatsNewDialog : public RoundedCornersDialog, protected Themable
{
    Q_OBJECT
public:
    explicit WhatsNewDialog(QWidget *parent = nullptr);

protected:
    void applyTheme(const Theme &theme) override;

private:
    QLabel *headerImageLabel;
    QLabel *headerLabel;
    QLabel *versionLabel;
    QLabel *textLabel;
    QPushButton *closeButton;
    QString htmlTemplate;
};
}

#endif // WHATSNEWDIALOG_H
