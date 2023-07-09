#include <QGridLayout>
#include <QSettings>
#include <QPalette>
#include <QBitmap>
#include <QPainter>
#include <QPixmap>

#include "server_config_dialog.h"
#include "yacreader_http_server.h"
#include "yacreader_global_gui.h"

#include "ip_config_helper.h"
#include "qrcodegen.hpp"

extern YACReaderHttpServer *httpServer;

ServerConfigDialog::ServerConfigDialog(QWidget *parent)
    : QDialog(parent)
{
    accept = new QPushButton(tr("set port"), this);
    qrCode = new QLabel(this);
    qrCode->move(64, 112);
    qrCode->setFixedSize(200, 200);
    qrCode->setScaledContents(true);

    QLabel *title1 = new QLabel(tr("Server connectivity information"), this);
    title1->move(332, 61);
    title1->setStyleSheet("QLabel {color:#474747; font-size:30px; font-family: Arial;}");

    QLabel *qrMessage = new QLabel(tr("Scan it!"), this);
    qrMessage->move(135, 388); // 373,627);
    qrMessage->setStyleSheet("QLabel {color:#A3A3A3; font-size:18px; font-family: Arial;}");
    qrMessage->setWordWrap(true);
    qrMessage->setFixedWidth(200);

    QLabel *propaganda = new QLabel(tr("YACReader is available for iOS and Android devices.<br/>Discover it for <a href='https://ios.yacreader.com' style='color:rgb(193, 148, 65)'>iOS</a> or <a href='https://android.yacreader.com' style='color:rgb(193, 148, 65)'>Android</a>."), this);
    propaganda->move(332, 505);
    propaganda->setStyleSheet("QLabel {color:#4D4D4D; font-size:13px; font-family: Arial; font-style: italic;}");
    propaganda->setOpenExternalLinks(true);

    // FORM---------------------------------------------------------------------

    QLabel *ipLabel = new QLabel(tr("Choose an IP address"), this);
    ipLabel->move(332, 117);
    ipLabel->setStyleSheet("QLabel {color:#575757; font-size:18px; font-family: Arial;}");

    QLabel *portLabel = new QLabel(tr("Port"), this);
    portLabel->move(332, 211);
    portLabel->setStyleSheet("QLabel {color:#575757; font-size:18px; font-family: Arial;}");

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
    check->setStyleSheet("QCheckBox {color:#262626; font-size:13px; font-family: Arial;}");

    QPalette palette;
    QImage image(":/images/serverConfigBackground.png");
    palette.setBrush(this->backgroundRole(), QBrush(image));

    setPalette(palette);

    this->setFixedSize(image.size());

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

    qrcodegen::QrCode code = qrcodegen::QrCode::encodeText(
            serverAddress.toLocal8Bit(),
            qrcodegen::QrCode::Ecc::LOW);

    QBitmap image(code.getSize(), code.getSize());
    image.fill();
    QPainter painter;
    painter.begin(&image);
    for (int x = 0; x < code.getSize(); x++) {
        for (int y = 0; y < code.getSize(); y++) {
            if (code.getModule(x, y)) {
                painter.drawPoint(x, y);
            }
        }
    }
    painter.end();

    image = image.scaled(qrCode->size() * devicePixelRatioF());

    QPixmap pMask(image.size());
    pMask.fill(QColor(66, 66, 66));
    pMask.setMask(image.createMaskFromColor(Qt::white));
    pMask.setDevicePixelRatio(devicePixelRatioF());

    qrCode->setPixmap(pMask);
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
