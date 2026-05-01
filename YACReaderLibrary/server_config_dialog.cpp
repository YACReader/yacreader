#include "server_config_dialog.h"

#include "ip_config_helper.h"
#include "qrcodegen.hpp"
#include "yacreader_global_gui.h"
#include "yacreader_http_server.h"

#include <QBitmap>
#include <QGridLayout>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QSettings>

extern YACReaderHttpServer *httpServer;

ServerConfigDialog::ServerConfigDialog(QWidget *parent)
    : QDialog(parent)
{
    // Background decoration (SVG on left side)
    backgroundDecoration = new QLabel(this);
    backgroundDecoration->move(0, 0);
    backgroundDecoration->setFixedSize(329, 595);
    backgroundDecoration->setScaledContents(true);

    accept = new QPushButton(tr("set port"), this);

    qrCode = new QLabel(this);
    qrCode->move(64, 112);
    qrCode->setFixedSize(200, 200);
    qrCode->setScaledContents(true);

    titleLabel = new QLabel(tr("Server connectivity information"), this);
    titleLabel->move(332, 61);

    qrMessageLabel = new QLabel(tr("Scan it!"), this);
    qrMessageLabel->move(135, 388);
    qrMessageLabel->setWordWrap(true);
    qrMessageLabel->setFixedWidth(200);

    propagandaLabel = new QLabel(tr("YACReader is available for iOS and Android devices.<br/>Discover it for <a href='https://ios.yacreader.com' style='color:rgb(193, 148, 65)'>iOS</a> or <a href='https://android.yacreader.com' style='color:rgb(193, 148, 65)'>Android</a>."), this);
    propagandaLabel->move(332, 505);
    propagandaLabel->setOpenExternalLinks(true);

    // FORM---------------------------------------------------------------------

    ipLabel = new QLabel(tr("Choose an IP address"), this);
    ipLabel->move(332, 117);

    portLabel = new QLabel(tr("Port"), this);
    portLabel->move(332, 211);

    ip = new QComboBox(this);
    connect(ip, &QComboBox::currentTextChanged, this, &ServerConfigDialog::regenerateQR);

    ip->setFixedWidth(200);
    ip->move(332, 153);

    port = new QLineEdit("8080", this);
    port->setReadOnly(false);

    connect(port, &QLineEdit::textChanged, this, [=](const QString &portValue) {
        accept->setEnabled(!portValue.isEmpty());
    });

    QValidator *validator = new QIntValidator(1024, 65535, this);
    port->setValidator(validator);

    QWidget *portWidget = new QWidget(this);
    auto portWidgetLayout = new QHBoxLayout(this);
    portWidgetLayout->addWidget(port);
    portWidgetLayout->addWidget(accept);
    portWidgetLayout->setContentsMargins(0, 0, 0, 0);
    portWidget->setLayout(portWidgetLayout);
    portWidget->move(332, 244);
    connect(accept, &QAbstractButton::pressed, this, &ServerConfigDialog::updatePort);

    // END FORM-----------------------------------------------------------------

    check = new QCheckBox(this);
    check->move(332, 314);
    check->setText(tr("enable the server"));

    this->setFixedSize(770, 595);

    QSettings *settings = new QSettings(YACReader::getSettingsPath() + "/YACReaderLibrary.ini", QSettings::IniFormat);
    settings->beginGroup("libraryConfig");

    if (settings->value(SERVER_ON, true).toBool()) {
        check->setChecked(true);
        generateQR();
    } else {
        ip->setDisabled(true);
        port->setDisabled(true);
        accept->setDisabled(true);

        check->setChecked(false);
    }

    settings->endGroup();

    connect(check, &QCheckBox::stateChanged, this, &ServerConfigDialog::enableServer);

    // Initialize theme
    initTheme(this);
}

void ServerConfigDialog::applyTheme(const Theme &theme)
{
    // Apply pre-built QSS from theme
    setStyleSheet(theme.serverConfigDialog.dialogQSS);
    titleLabel->setStyleSheet(theme.serverConfigDialog.titleLabelQSS);
    qrMessageLabel->setStyleSheet(theme.serverConfigDialog.qrMessageLabelQSS);
    propagandaLabel->setStyleSheet(theme.serverConfigDialog.propagandaLabelQSS);
    ipLabel->setStyleSheet(theme.serverConfigDialog.labelQSS);
    portLabel->setStyleSheet(theme.serverConfigDialog.labelQSS);
    check->setStyleSheet(theme.serverConfigDialog.checkBoxQSS);

    // Set background decoration (SVG on left)
    backgroundDecoration->setPixmap(theme.serverConfigDialog.backgroundDecoration);
    backgroundDecoration->lower(); // Send to back so other widgets appear on top

    // Regenerate QR code with new theme colors
    generateQR();
}

void ServerConfigDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);

    generateQR();
}

void ServerConfigDialog::enableServer(int status)
{
    QSettings *settings = new QSettings(YACReader::getSettingsPath() + "/YACReaderLibrary.ini", QSettings::IniFormat);
    settings->beginGroup("libraryConfig");

    if (status == Qt::Checked) {
        ip->setDisabled(false);
        port->setDisabled(false);
        accept->setDisabled(false);
        httpServer->start();
        this->generateQR();
        settings->setValue(SERVER_ON, true);
    } else {
        httpServer->stop();
        qrCode->setPixmap(QPixmap());
        ip->setDisabled(true);
        port->setDisabled(true);
        accept->setDisabled(true);
        settings->setValue(SERVER_ON, false);
    }
    settings->endGroup();
}

void ServerConfigDialog::generateQR()
{
    if (!httpServer->isRunning())
        return;

    ip->clear();

    auto addresses = getIpAddresses();
    if (addresses.length() > 0) {
        generateQR(addresses.first() + ":" + httpServer->getPort());
        ip->addItems(addresses);
        port->setText(httpServer->getPort());
    }
}

void ServerConfigDialog::generateQR(const QString &serverAddress)
{
    qrCode->clear();

    auto backgroundColor = theme.serverConfigDialog.qrBackgroundColor;
    auto foregroundColor = theme.serverConfigDialog.qrForegroundColor;

    qrcodegen::QrCode code = qrcodegen::QrCode::encodeText(
            serverAddress.toLocal8Bit(),
            qrcodegen::QrCode::Ecc::LOW);

    int qrSize = code.getSize();
    QPixmap qrPixmap(qrSize, qrSize);
    qrPixmap.fill(backgroundColor);

    QPainter painter(&qrPixmap);
    painter.setPen(foregroundColor);
    painter.setBrush(foregroundColor);
    for (int x = 0; x < qrSize; x++) {
        for (int y = 0; y < qrSize; y++) {
            if (code.getModule(x, y)) {
                painter.drawPoint(x, y);
            }
        }
    }
    painter.end();

    // Scale to label size
    qrPixmap = qrPixmap.scaled(qrCode->size() * devicePixelRatioF(), Qt::KeepAspectRatio, Qt::FastTransformation);
    qrPixmap.setDevicePixelRatio(devicePixelRatioF());

    qrCode->setPixmap(qrPixmap);
}

void ServerConfigDialog::regenerateQR(const QString &ip)
{
    generateQR(ip + ":" + httpServer->getPort());
}

void ServerConfigDialog::updatePort()
{
    QSettings *settings = new QSettings(YACReader::getSettingsPath() + "/YACReaderLibrary.ini", QSettings::IniFormat); // TODO unificar la creación del fichero de config con el servidor
    settings->beginGroup("listener");
    settings->setValue("port", port->text().toInt());
    settings->endGroup();

    httpServer->stop();
    httpServer->start();

    generateQR(ip->currentText() + ":" + port->text());
}
