#include "server_config_dialog.h"

#include "ip_config_helper.h"
#include "qrcodegen.hpp"
#include "yacreader_global_gui.h"
#include "yacreader_http_server.h"

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QFrame>
#include <QGridLayout>
#include <QHostAddress>
#include <QHostInfo>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QNetworkInterface>
#include <QPainter>
#include <QPalette>
#include <QSettings>
#include <QSignalBlocker>
#include <QUrl>
#include <QVBoxLayout>

extern YACReaderHttpServer *httpServer;

ServerConfigDialog::ServerConfigDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Server connectivity"));
    setFixedSize(770, 595);

    backgroundDecoration = new QLabel(this);
    backgroundDecoration->setGeometry(0, 0, 329, 595);
    backgroundDecoration->setScaledContents(true);

    qrCode = new QLabel(this);
    qrCode->setGeometry(64, 112, 200, 200);
    qrCode->setScaledContents(true);

    qrMessageLabel = new QLabel(tr("Scan to connect"), this);
    qrMessageLabel->setGeometry(43, 312, 243, 208);
    qrMessageLabel->setAlignment(Qt::AlignCenter);
    qrMessageLabel->setWordWrap(true);

    auto detailsWidget = new QWidget(this);
    detailsWidget->setGeometry(332, 0, 410, 595);

    auto detailsLayout = new QVBoxLayout(detailsWidget);
    detailsLayout->setContentsMargins(0, 48, 0, 43);
    detailsLayout->setSpacing(0);

    titleLabel = new QLabel(tr("Server connectivity"), detailsWidget);
    detailsLayout->addWidget(titleLabel);
    detailsLayout->addSpacing(7);

    descriptionLabel = new QLabel(tr("Devices on this network can reach your library at the address below."), detailsWidget);
    descriptionLabel->setWordWrap(true);
    detailsLayout->addWidget(descriptionLabel);
    detailsLayout->addSpacing(25);

    auto formLayout = new QGridLayout;
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->setHorizontalSpacing(0);
    formLayout->setVerticalSpacing(7);
    formLayout->setColumnStretch(0, 9);
    formLayout->setColumnMinimumWidth(1, 16);
    formLayout->setColumnStretch(2, 4);
    formLayout->setColumnMinimumWidth(3, 7);

    ipLabel = new QLabel(tr("IP address"), detailsWidget);
    portLabel = new QLabel(tr("Port"), detailsWidget);
    formLayout->addWidget(ipLabel, 0, 0);
    formLayout->addWidget(portLabel, 0, 2);

    ip = new QComboBox(detailsWidget);
    ip->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    ip->setMinimumWidth(100);
    // Editable: besides the detected addresses, an IP or a host name can be typed
    ip->setEditable(true);
    ip->setInsertPolicy(QComboBox::NoInsert);

    pinButton = new QPushButton(detailsWidget);
    pinButton->setCheckable(true);
    pinButton->setCursor(Qt::PointingHandCursor);
    pinButton->setFocusPolicy(Qt::TabFocus);
    port = new QLineEdit(QStringLiteral("8080"), detailsWidget);
    port->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    port->setMinimumWidth(55);
    port->setValidator(new QIntValidator(1024, 65535, this));
    accept = new QPushButton(tr("Set port"), detailsWidget);
    accept->setObjectName(QStringLiteral("primaryButton"));
    accept->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    auto ipLayout = new QHBoxLayout;
    ipLayout->setContentsMargins(0, 0, 0, 0);
    ipLayout->setSpacing(7);
    ipLayout->addWidget(ip);
    ipLayout->addWidget(pinButton);

    formLayout->addLayout(ipLayout, 1, 0);
    formLayout->addWidget(port, 1, 2);
    formLayout->addWidget(accept, 1, 4);

    pinWarningLabel = new QLabel(tr("This address is not assigned to this computer on the current network."), detailsWidget);
    pinWarningLabel->setWordWrap(true);
    pinWarningLabel->hide();
    formLayout->addWidget(pinWarningLabel, 2, 0, 1, 5);
    detailsLayout->addLayout(formLayout);
    detailsLayout->addSpacing(26);

    connectionCard = new QFrame(detailsWidget);
    connectionCard->setObjectName(QStringLiteral("connectionCard"));
    auto cardLayout = new QVBoxLayout(connectionCard);
    cardLayout->setContentsMargins(16, 14, 16, 14);
    cardLayout->setSpacing(5);

    webInterfaceLabel = new QLabel(tr("Web interface").toUpper(), connectionCard);
    webInterfaceLabel->setObjectName(QStringLiteral("sectionLabel"));
    cardLayout->addWidget(webInterfaceLabel);

    webInterfaceUrlLabel = new QLabel(connectionCard);
    webInterfaceUrlLabel->setObjectName(QStringLiteral("webInterfaceUrl"));
    webInterfaceUrlLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    cardLayout->addWidget(webInterfaceUrlLabel);
    cardLayout->addSpacing(7);

    auto buttonsLayout = new QHBoxLayout;
    buttonsLayout->setContentsMargins(0, 0, 0, 0);
    buttonsLayout->setSpacing(8);
    buttonsLayout->addStretch();

    copyLinkButton = new QPushButton(tr("Copy link"), connectionCard);
    copyLinkButton->setObjectName(QStringLiteral("secondaryButton"));
    openWebUiButton = new QPushButton(tr("Open web UI"), connectionCard);
    openWebUiButton->setObjectName(QStringLiteral("primaryButton"));
    buttonsLayout->addWidget(copyLinkButton);
    buttonsLayout->addWidget(openWebUiButton);
    cardLayout->addLayout(buttonsLayout);
    detailsLayout->addWidget(connectionCard);
    detailsLayout->addSpacing(25);

    check = new QCheckBox(tr("Enable the server"), detailsWidget);
    detailsLayout->addWidget(check);
    detailsLayout->addStretch();

    propagandaText = tr("YACReader is available for iOS and Android. Discover it for <a href='https://ios.yacreader.com'>iOS</a> or <a href='https://android.yacreader.com'>Android</a>.");
    propagandaLabel = new QLabel(detailsWidget);
    propagandaLabel->setOpenExternalLinks(true);
    propagandaLabel->setWordWrap(true);
    detailsLayout->addWidget(propagandaLabel);

    connect(ip, &QComboBox::currentTextChanged, this, &ServerConfigDialog::regenerateQR);
    connect(pinButton, &QPushButton::toggled, this, &ServerConfigDialog::setPinned);
    connect(port, &QLineEdit::textChanged, this, [this] {
        accept->setEnabled(check->isChecked() && port->hasAcceptableInput() && port->text() != httpServer->getPort());
    });
    connect(accept, &QPushButton::clicked, this, &ServerConfigDialog::updatePort);
    connect(copyLinkButton, &QPushButton::clicked, this, [this] {
        QApplication::clipboard()->setText(webInterfaceUrl());
    });
    connect(openWebUiButton, &QPushButton::clicked, this, [this] {
        QDesktopServices::openUrl(QUrl(webInterfaceUrl()));
    });

    QSettings settings(YACReader::getSettingsPath() + "/YACReaderLibrary.ini", QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("libraryConfig"));
    const bool serverEnabled = settings.value(SERVER_ON, true).toBool();
    pinnedAddress = settings.value(SERVER_PINNED_ADDRESS).toString().trimmed();
    // A hand-edited or corrupted value must not end up in the QR code
    if (!isValidServerHost(pinnedAddress) && QHostAddress(pinnedAddress).isNull())
        pinnedAddress.clear();
    settings.endGroup();
    {
        const QSignalBlocker pinBlocker(pinButton);
        pinButton->setChecked(!pinnedAddress.isEmpty());
    }

    initTheme(this);
    accept->setMinimumWidth(qMax(102, accept->sizeHint().width()));

    check->setChecked(serverEnabled);
    connect(check, &QCheckBox::stateChanged, this, &ServerConfigDialog::enableServer);
    setConnectionControlsEnabled(serverEnabled);
    if (serverEnabled)
        generateQR();
    else {
        qrCode->clear();
        refreshWebInterface();
    }
}

void ServerConfigDialog::applyTheme(const Theme &theme)
{
    setStyleSheet(theme.serverConfigDialog.dialogQSS);
    titleLabel->setStyleSheet(theme.serverConfigDialog.titleLabelQSS);
    qrMessageLabel->setStyleSheet(theme.serverConfigDialog.qrMessageLabelQSS);
    propagandaLabel->setStyleSheet(theme.serverConfigDialog.propagandaLabelQSS);
    const QColor linkColor = theme.serverConfigDialog.linkColor;
    QString themedPropagandaText = propagandaText;
    themedPropagandaText.replace(QStringLiteral("<a "), QStringLiteral("<a style='color:%1' ").arg(linkColor.name()));
    propagandaLabel->setText(themedPropagandaText);
    auto propagandaPalette = propagandaLabel->palette();
    propagandaPalette.setColor(QPalette::Link, linkColor);
    propagandaPalette.setColor(QPalette::LinkVisited, linkColor);
    propagandaLabel->setPalette(propagandaPalette);
    descriptionLabel->setStyleSheet(theme.serverConfigDialog.textLabelQSS);
    ipLabel->setStyleSheet(theme.serverConfigDialog.secondaryLabelQSS);
    portLabel->setStyleSheet(theme.serverConfigDialog.secondaryLabelQSS);
    check->setStyleSheet(theme.serverConfigDialog.checkBoxQSS);
    pinButton->setStyleSheet(theme.serverConfigDialog.pinButtonQSS);
    pinWarningLabel->setStyleSheet(theme.serverConfigDialog.secondaryLabelQSS);

    backgroundDecoration->setPixmap(theme.serverConfigDialog.backgroundDecoration);
    backgroundDecoration->lower();

    generateQR();
}

void ServerConfigDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    generateQR();
    refreshHostNameCandidates();
}

void ServerConfigDialog::enableServer(int status)
{
    QSettings settings(YACReader::getSettingsPath() + "/YACReaderLibrary.ini", QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("libraryConfig"));

    const bool enabled = status == Qt::Checked;
    setConnectionControlsEnabled(enabled);
    if (enabled) {
        httpServer->start();
        generateQR();
    } else {
        httpServer->stop();
        qrCode->clear();
        refreshWebInterface();
    }
    settings.setValue(SERVER_ON, enabled);
    settings.endGroup();
}

void ServerConfigDialog::generateQR()
{
    if (!httpServer->isRunning()) {
        refreshWebInterface();
        return;
    }

    const QSignalBlocker blocker(ip);
    // Keep what the user chose instead of falling back to the first address every time
    const QString selected = ip->currentText().trimmed();
    availableAddresses = getIpAddresses();
    // Computer names go last: the first IP stays the default, but a name is one click away
    for (const auto &hostName : std::as_const(verifiedHostNames)) {
        if (!availableAddresses.contains(hostName))
            availableAddresses.append(hostName);
    }
    ip->clear();
    ip->addItems(availableAddresses);
    port->setText(httpServer->getPort());

    const QString address = resolveServerAddress(pinnedAddress, availableAddresses, selected);
    const int index = ip->findText(address);
    if (index >= 0)
        ip->setCurrentIndex(index);
    else
        ip->setEditText(address);

    updateIpControls();
    updatePinWarning();

    if (!address.isEmpty())
        generateQR(address + ":" + httpServer->getPort());
    else
        refreshWebInterface();
}

void ServerConfigDialog::generateQR(const QString &serverAddress)
{
    qrCode->clear();

    const auto backgroundColor = theme.serverConfigDialog.qrBackgroundColor;
    const auto foregroundColor = theme.serverConfigDialog.qrForegroundColor;
    const qrcodegen::QrCode code = qrcodegen::QrCode::encodeText(
            serverAddress.toLocal8Bit(),
            qrcodegen::QrCode::Ecc::LOW);

    const int qrSize = code.getSize();
    QPixmap qrPixmap(qrSize, qrSize);
    qrPixmap.fill(backgroundColor);

    QPainter painter(&qrPixmap);
    painter.setPen(foregroundColor);
    painter.setBrush(foregroundColor);
    for (int x = 0; x < qrSize; ++x) {
        for (int y = 0; y < qrSize; ++y) {
            if (code.getModule(x, y))
                painter.drawPoint(x, y);
        }
    }
    painter.end();

    qrPixmap = qrPixmap.scaled(qrCode->size() * devicePixelRatioF(), Qt::KeepAspectRatio, Qt::FastTransformation);
    qrPixmap.setDevicePixelRatio(devicePixelRatioF());
    qrCode->setPixmap(qrPixmap);
    refreshWebInterface();
}

void ServerConfigDialog::regenerateQR(const QString &text)
{
    const QString address = text.trimmed();
    updateIpControls();
    if (httpServer->isRunning() && isUsableAddress(address))
        generateQR(address + ":" + httpServer->getPort());
    else
        refreshWebInterface();
}

void ServerConfigDialog::updatePort()
{
    if (!port->hasAcceptableInput()) {
        port->setText(httpServer->getPort());
        return;
    }
    if (port->text() == httpServer->getPort())
        return;

    QSettings settings(YACReader::getSettingsPath() + "/YACReaderLibrary.ini", QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("listener"));
    settings.setValue(QStringLiteral("port"), port->text().toInt());
    settings.endGroup();

    httpServer->stop();
    httpServer->start();
    generateQR();
}

QString ServerConfigDialog::webInterfaceUrl() const
{
    const QString address = currentAddress();
    if (!isUsableAddress(address))
        return { };
    return QStringLiteral("http://%1:%2/webui").arg(address, httpServer->getPort());
}

void ServerConfigDialog::refreshWebInterface()
{
    const QString url = httpServer->isRunning() ? webInterfaceUrl() : QString();
    webInterfaceUrlLabel->setText(url.isEmpty() ? QStringLiteral("—") : url);
    copyLinkButton->setEnabled(!url.isEmpty());
    openWebUiButton->setEnabled(!url.isEmpty());
}

void ServerConfigDialog::setConnectionControlsEnabled(bool enabled)
{
    updateIpControls();
    port->setEnabled(enabled);
    accept->setEnabled(enabled && port->hasAcceptableInput() && port->text() != httpServer->getPort());
    if (!enabled) {
        copyLinkButton->setEnabled(false);
        openWebUiButton->setEnabled(false);
    }
}

QString ServerConfigDialog::currentAddress() const
{
    return ip->currentText().trimmed();
}

bool ServerConfigDialog::isUsableAddress(const QString &address) const
{
    // Detected addresses are always valid (they can be IPv6); typed ones must be an IPv4 or a host name
    return !address.isEmpty() && (availableAddresses.contains(address) || isValidServerHost(address));
}

void ServerConfigDialog::setPinned(bool pinned)
{
    if (pinned) {
        const QString address = currentAddress();
        if (!isUsableAddress(address)) {
            // Nothing valid to pin: undo the toggle without re-entering this slot
            const QSignalBlocker blocker(pinButton);
            pinButton->setChecked(false);
            return;
        }
        pinnedAddress = address;
    } else {
        pinnedAddress.clear();
    }

    QSettings settings(YACReader::getSettingsPath() + "/YACReaderLibrary.ini", QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("libraryConfig"));
    if (pinnedAddress.isEmpty())
        settings.remove(SERVER_PINNED_ADDRESS);
    else
        settings.setValue(SERVER_PINNED_ADDRESS, pinnedAddress);
    settings.endGroup();

    updateIpControls();
    generateQR();
}

void ServerConfigDialog::updateIpControls()
{
    const bool pinned = !pinnedAddress.isEmpty();
    const bool serverEnabled = check->isChecked();

    // While pinned the address is locked: unpin to change it
    ip->setEnabled(serverEnabled && !pinned);
    pinButton->setEnabled(serverEnabled && (pinned || isUsableAddress(currentAddress())));
    pinButton->setToolTip(pinned ? tr("Unpin this address") : tr("Pin this address so it never changes"));
}

void ServerConfigDialog::updatePinWarning()
{
    // Only an IPv4 can be checked against the network interfaces; a host name cannot
    QHostAddress pinnedIp;
    const bool pinnedIsIpv4 = pinnedIp.setAddress(pinnedAddress) && pinnedIp.protocol() == QAbstractSocket::IPv4Protocol;
    pinWarningLabel->setVisible(httpServer->isRunning() && pinnedIsIpv4 && !availableAddresses.contains(pinnedAddress));
}

void ServerConfigDialog::refreshHostNameCandidates()
{
    const auto candidates = localHostNameCandidates(QHostInfo::localHostName(), QHostInfo::localDomainName());

    // Names are only offered if this computer can resolve them to one of its own addresses,
    // which is the best check available without asking another device. The lookups are
    // asynchronous (a name that is not announced can take seconds to fail) and tied to the
    // dialog, so they are dropped if it is closed.
    for (const auto &candidate : candidates) {
        QHostInfo::lookupHost(candidate, this, [this, candidate](const QHostInfo &info) {
            const bool reachable = info.error() == QHostInfo::NoError
                    && resolvesToLocalAddress(info.addresses(), QNetworkInterface::allAddresses());

            if (!reachable) {
                verifiedHostNames.removeAll(candidate);
                return;
            }
            if (verifiedHostNames.contains(candidate))
                return;
            verifiedHostNames.append(candidate);

            // Add it to the list without touching what the user is typing or has selected
            if (!availableAddresses.contains(candidate) && httpServer->isRunning()) {
                availableAddresses.append(candidate);
                const QSignalBlocker blocker(ip);
                const QString current = ip->currentText();
                ip->addItem(candidate);
                ip->setEditText(current);
                updateIpControls();
            }
        });
    }
}
