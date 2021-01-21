#include "whats_new_controller.h"

#include "whats_new_dialog.h"
#include "yacreader_global.h"

#include <QtCore>

YACReader::WhatsNewController::WhatsNewController() { }

void YACReader::WhatsNewController::showWhatsNewIfNeeded(QWidget *fromParent)
{
    QSettings commonSettings(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
                                     "/YACReader/YACReaderCommon.ini",
                             QSettings::IniFormat);

    if (commonSettings.value("LAST_VERSION_INSTALLED").toString() != VERSION) {
        showWhatsNew(fromParent);
        commonSettings.setValue("LAST_VERSION_INSTALLED", VERSION);
    }
}

void YACReader::WhatsNewController::showWhatsNew(QWidget *fromParent)
{
    auto whatsNewDialog = new WhatsNewDialog(fromParent);

    whatsNewDialog->show();

    QObject::connect(whatsNewDialog, &QDialog::finished, whatsNewDialog, &QDialog::deleteLater);
}
