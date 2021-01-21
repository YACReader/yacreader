#ifndef WHATSNEWDIALOG_H
#define WHATSNEWDIALOG_H

#include "rounded_corners_dialog.h"
#include <QObject>

namespace YACReader {

class WhatsNewDialog : public RoundedCornersDialog
{
    Q_OBJECT
public:
    explicit WhatsNewDialog(QWidget *parent = nullptr);
};
}

#endif // WHATSNEWDIALOG_H
