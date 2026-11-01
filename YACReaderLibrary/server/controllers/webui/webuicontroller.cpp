#include "webuicontroller.h"

#include "db_helper.h"
#include "qrcodegen.hpp"
#include "template.h"
#include "yacreader_global.h"
#include "yacreader_libraries.h"

#include <QCoreApplication>
#include <QDir>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QRegularExpression>
#include <QSettings>
#include <QSysInfo>
#include <QTime>
#include <QUrl>
#include <QUuid>

#include <algorithm>

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;
using stefanfrings::Template;

namespace {

const QByteArray &settingsCsrfToken()
{
    static const QByteArray token = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();
    return token;
}

QString settingsFilePath()
{
    return QDir(YACReader::getSettingsPath()).filePath(QCoreApplication::applicationName() + ".ini");
}

QString requestPort(const HttpRequest &request)
{
    const QUrl requestUrl(QStringLiteral("http://") + QString::fromUtf8(request.getHeader("host")));
    const int port = requestUrl.port();
    return port > 0 ? QString::number(port) : QString(QChar(0x2014));
}

// "host:port" addresses that YACReader for iOS and Android can use to reach this server,
// in the same format the desktop server dialog encodes in its QR code.
QStringList connectAddresses(const HttpRequest &request)
{
    const QUrl requestUrl(QStringLiteral("http://") + QString::fromUtf8(request.getHeader("host")));
    const QString port = QString::number(requestUrl.port(80));

    QStringList hosts;

    // The address this browser used is known to reach the server, so it goes first.
    const QHostAddress requestAddress(requestUrl.host());
    if (requestAddress.protocol() == QAbstractSocket::IPv4Protocol && !requestAddress.isLoopback()) {
        hosts.append(requestAddress.toString());
    }

    // Same filter as the desktop dialog (getIpAddresses), limited to IPv4 because
    // "host:port" is ambiguous for IPv6 addresses.
    QStringList interfaceHosts;
    const auto allAddresses = QNetworkInterface::allAddresses();
    for (const QHostAddress &address : allAddresses) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address.isGlobal()) {
            interfaceHosts.append(address.toString());
        }
    }
    std::stable_partition(interfaceHosts.begin(), interfaceHosts.end(), [](const QString &host) {
        return host.startsWith(QStringLiteral("192.168."));
    });

    for (const QString &host : std::as_const(interfaceHosts)) {
        if (!hosts.contains(host)) {
            hosts.append(host);
        }
    }

    QStringList addresses;
    for (const QString &host : std::as_const(hosts)) {
        addresses.append(host + QLatin1Char(':') + port);
    }
    return addresses;
}

// Inline SVG, so it works with the page CSP (no data: URLs) and needs no extra request.
QString qrCodeSvg(const QString &text)
{
    const qrcodegen::QrCode code = qrcodegen::QrCode::encodeText(text.toUtf8().constData(), qrcodegen::QrCode::Ecc::LOW);
    const int border = 4;
    const int size = code.getSize();

    QString path;
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            if (code.getModule(x, y)) {
                path += QStringLiteral("M%1 %2h1v1h-1z").arg(x + border).arg(y + border);
            }
        }
    }

    return QStringLiteral(R"(<svg class="connect-qr-image" viewBox="0 0 %1 %1" role="img" aria-label="QR code for %2" shape-rendering="crispEdges"><rect width="%1" height="%1" fill="#fff"/><path fill="#000" d="%3"/></svg>)")
            .arg(size + border * 2)
            .arg(text.toHtmlEscaped(), path);
}

QString checkedAttribute(bool checked)
{
    return checked ? QStringLiteral("checked") : QString();
}

QString selectedAttribute(int selectedValue, int value)
{
    return selectedValue == value ? QStringLiteral("selected") : QString();
}

void setPageHeaders(HttpResponse &response)
{
    response.setHeader("Content-Type", "text/html; charset=utf-8");
    response.setHeader("Connection", "close");
    response.setHeader("Cache-Control", "no-store");
    response.setHeader("Content-Security-Policy", "default-src 'self'; img-src 'self' blob:; style-src 'self'; script-src 'self'; base-uri 'none'; form-action 'self'; frame-ancestors 'none'");
    response.setHeader("Referrer-Policy", "no-referrer");
    response.setHeader("X-Content-Type-Options", "nosniff");
    response.setHeader("X-Frame-Options", "DENY");
}

void methodNotAllowed(HttpResponse &response, const QByteArray &allowedMethods)
{
    response.setStatus(405, "Method Not Allowed");
    response.setHeader("Allow", allowedMethods);
    response.setHeader("Content-Type", "text/plain; charset=utf-8");
    response.write("405 method not allowed", true);
}

} // namespace

WebUIController::WebUIController() { }

void WebUIController::service(HttpRequest &request, HttpResponse &response)
{
    const QByteArray path = request.getPath();
    const QByteArray method = request.getMethod();

    if (path == "/") {
        response.redirect("/webui");
        return;
    }

    if (path == "/webui" || path == "/webui/") {
        if (method != "GET") {
            methodNotAllowed(response, "GET");
            return;
        }

        renderStatusPage(request, response);
        return;
    }

    const QRegularExpression libraryBrowserPath(
            QStringLiteral(R"(^/webui/library/([0-9]+)(?:/(folder|comic|reading_list_folder|reading_list|smart_list_folder|smart_list|tag)/([0-9]+)(?:/(read)|/(missing)/[0-9]+)?|/(reading_lists|smart_lists|tags|favorites|recent))?/?$)"));
    const QRegularExpressionMatch libraryBrowserMatch = libraryBrowserPath.match(QString::fromUtf8(path));
    if (libraryBrowserMatch.hasMatch()) {
        if (method != "GET") {
            methodNotAllowed(response, "GET");
            return;
        }

        bool libraryIdIsValid = false;
        const int libraryId = libraryBrowserMatch.captured(1).toInt(&libraryIdIsValid);
        const YACReaderLibraries libraries = DBHelper::getLibraries();
        if (!libraryIdIsValid || !libraries.contains(libraryId)) {
            response.setStatus(404, "Not Found");
            response.setHeader("Content-Type", "text/plain; charset=utf-8");
            response.write("404 library not found", true);
            return;
        }

        if (!libraryBrowserMatch.captured(4).isEmpty() && libraryBrowserMatch.captured(2) != QStringLiteral("comic")) {
            response.setStatus(404, "Not Found");
            response.setHeader("Content-Type", "text/plain; charset=utf-8");
            response.write("404 invalid reader path", true);
            return;
        }
        if (!libraryBrowserMatch.captured(5).isEmpty() && libraryBrowserMatch.captured(2) != QStringLiteral("folder") && libraryBrowserMatch.captured(2) != QStringLiteral("reading_list") && libraryBrowserMatch.captured(2) != QStringLiteral("tag")) {
            response.setStatus(404, "Not Found");
            response.setHeader("Content-Type", "text/plain; charset=utf-8");
            response.write("404 invalid missing comic path", true);
            return;
        }

        const QString initialView = !libraryBrowserMatch.captured(4).isEmpty()
                ? QStringLiteral("reader")
                : !libraryBrowserMatch.captured(5).isEmpty()
                ? QStringLiteral("missing")
                : !libraryBrowserMatch.captured(6).isEmpty()
                ? libraryBrowserMatch.captured(6)
                : libraryBrowserMatch.captured(2).isEmpty()
                ? QStringLiteral("folder")
                : libraryBrowserMatch.captured(2);
        bool itemIdIsValid = false;
        qulonglong initialItemId = libraryBrowserMatch.captured(3).toULongLong(&itemIdIsValid);
        if (libraryBrowserMatch.captured(3).isEmpty()) {
            initialItemId = 1;
            itemIdIsValid = true;
        }

        if (!itemIdIsValid) {
            response.setStatus(404, "Not Found");
            response.setHeader("Content-Type", "text/plain; charset=utf-8");
            response.write("404 item not found", true);
            return;
        }

        renderLibraryBrowser(request, response, libraryId, libraries.getName(libraryId), initialView, initialItemId);
        return;
    }

    if (path == "/webui/settings" || path == "/webui/settings/") {
        if (method == "GET") {
            renderSettingsPage(request, response);
            return;
        }

        if (method == "POST") {
            QString errorMessage;
            int errorStatus = 400;
            if (saveSettings(request, errorMessage, errorStatus)) {
                response.redirect("/webui/settings?saved=1");
            } else {
                QByteArray statusDescription = "Bad Request";
                if (errorStatus == 403) {
                    statusDescription = "Forbidden";
                } else if (errorStatus == 500) {
                    statusDescription = "Internal Server Error";
                }
                response.setStatus(errorStatus, statusDescription);
                renderSettingsPage(request, response, errorMessage);
            }
            return;
        }

        methodNotAllowed(response, "GET, POST");
        return;
    }

    response.setStatus(404, "Not Found");
    response.setHeader("Content-Type", "text/plain; charset=utf-8");
    response.write("404 not found", true);
}

void WebUIController::renderStatusPage(HttpRequest &request, HttpResponse &response)
{
    setPageHeaders(response);

    Template statusPage(
            QStringLiteral(R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <meta name="color-scheme" content="light dark">
  <title>Home · YACReaderLibrary</title>
  <link rel="stylesheet" href="/css/webui.css">
  <script src="/js/webui.js"></script>
</head>
<body>
  <div class="app-shell">
    <aside class="sidebar">
      <div class="sidebar-header">
        <a class="brand" href="/webui">
          <img class="brand-logo" src="/images/webui/YACLibraryServer.svg" alt="">
          <span class="brand-copy">
            <span class="brand-name">YACReader</span>
            <span class="brand-product">Library Server</span>
          </span>
        </a>
        <button class="theme-toggle" data-theme-toggle type="button" aria-label="Use dark theme">
          <svg class="moon" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
            <path d="M21 12.8A9 9 0 1 1 11.2 3 7 7 0 0 0 21 12.8z"/>
          </svg>
          <svg class="sun" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
            <circle cx="12" cy="12" r="4"/>
            <path d="M12 2v2M12 20v2M4.93 4.93l1.42 1.42M17.66 17.66l1.41 1.41M2 12h2M20 12h2M4.93 19.07l1.42-1.42M17.66 6.34l1.41-1.41"/>
          </svg>
        </button>
      </div>

      <nav class="navigation" aria-label="Server">
        <a class="navigation-item active" href="/webui" aria-current="page">
          <svg width="18" height="18" viewBox="0 0 18 18" fill="none" aria-hidden="true">
            <circle cx="9" cy="9" r="6.4" stroke="currentColor" stroke-width="1.6"/>
            <circle cx="9" cy="9" r="2.6" fill="currentColor"/>
          </svg>
          Home
        </a>
        <a class="navigation-item" href="/webui/settings">
          <svg width="18" height="18" viewBox="0 0 18 18" fill="currentColor" aria-hidden="true">
            <rect x="2.4" y="4.1" width="13.2" height="1.6" rx=".8"/>
            <circle cx="12" cy="4.9" r="2.4"/>
            <rect x="2.4" y="11.4" width="13.2" height="1.6" rx=".8"/>
            <circle cx="6" cy="12.2" r="2.4"/>
          </svg>
          Settings
        </a>
      </nav>

      <div class="server-summary">
        <span class="online-dot" aria-hidden="true"></span>
        <span>Server {server.version} · :{os.port}</span>
      </div>
    </aside>

    <main class="main">
      <header class="topbar">
        <div>
          <div class="eyebrow">Library Server</div>
          <h1 class="page-title">Home</h1>
        </div>
        {if connect.available}
        <button class="ghost-button topbar-action" type="button" data-connect-open aria-haspopup="dialog">
          <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
            <rect x="6.5" y="2.5" width="11" height="19" rx="2.5"/>
            <path d="M10 5h4M11 18.5h2"/>
          </svg>
          <span>Connect a device</span>
        </button>
        {end connect.available}
      </header>

      <div class="content">
        <section class="section" id="libraries" aria-labelledby="libraries-heading" data-update-libraries>
          <div class="section-header">
            <div class="section-header-main">
              <div class="section-title" id="libraries-heading">Libraries</div>
              <span class="count">{library.count}</span>
            </div>
            <button class="ghost-button update-all" type="button" data-update-all aria-label="Update all libraries">
              <span class="update-icon" aria-hidden="true">
                <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                  <polyline points="23 4 23 10 17 10"/>
                  <polyline points="1 20 1 14 7 14"/>
                  <path d="M3.51 9a9 9 0 0 1 14.85-3.36L23 10M1 14l4.64 4.36A9 9 0 0 0 20.49 15"/>
                </svg>
              </span>
              <span class="update-spinner" aria-hidden="true"></span>
              <span>Update all</span>
            </button>
          </div>
          <p class="library-description">Click a library to browse it · updating rescans its contents from disk.</p>
          <div class="library-grid">
            {loop Library}
            <div class="library-card" data-library-card data-library-id="{Library.Id}">
              <div class="library-initial" aria-hidden="true">{Library.Initial}</div>
              <div class="library-copy">
                <a class="library-card-link" href="/webui/library/{Library.Id}" title="{Library.Name}">{Library.Name}</a>
                <div class="library-path" title="{Library.Path}">{Library.Path}</div>
              </div>
              <div class="library-actions">
              <div class="library-menu" data-menu data-lock-notice hidden>
                <button class="icon-button library-menu-toggle library-notice-toggle" type="button" data-menu-toggle aria-haspopup="dialog" aria-expanded="false" aria-label="Update problem for {Library.Name}">
                  <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.2" stroke-linecap="round" aria-hidden="true">
                    <circle cx="12" cy="12" r="9.5"/>
                    <line x1="12" y1="7" x2="12" y2="13"/>
                    <circle cx="12" cy="16.6" r="0.6" fill="currentColor"/>
                  </svg>
                </button>
                <div class="library-menu-popover library-notice-popover" data-menu-popover role="dialog" aria-label="Update problem for {Library.Name}" hidden>
                  <h3 data-lock-notice-title></h3>
                  <p data-lock-notice-text></p>
                  <p class="notice-warning" data-lock-notice-warning hidden>Before you recover, make sure no YACReader app is using this library, also on another computer.</p>
                  <p class="notice-owner" data-lock-notice-owner></p>
                  <p class="notice-error" data-lock-notice-error role="status" hidden></p>
                  <button class="primary-button" type="button" data-lock-notice-action></button>
                </div>
              </div>
              <div class="library-menu" data-menu>
                <button class="icon-button library-menu-toggle" type="button" data-menu-toggle aria-haspopup="true" aria-expanded="false" aria-label="Actions for {Library.Name}">
                  <svg width="18" height="18" viewBox="0 0 24 24" fill="currentColor" aria-hidden="true">
                    <circle cx="5" cy="12" r="1.7"/>
                    <circle cx="12" cy="12" r="1.7"/>
                    <circle cx="19" cy="12" r="1.7"/>
                  </svg>
                </button>
                <div class="library-menu-popover" data-menu-popover role="menu" hidden>
                  <button class="menu-item" type="button" role="menuitem" data-update-library="{Library.Id}">
                    <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                      <polyline points="23 4 23 10 17 10"/>
                      <polyline points="1 20 1 14 7 14"/>
                      <path d="M3.51 9a9 9 0 0 1 14.85-3.36L23 10M1 14l4.64 4.36A9 9 0 0 0 20.49 15"/>
                    </svg>
                    <span>Update library</span>
                  </button>
                  <button class="menu-item" type="button" role="menuitem" data-rescan-xml-library="{Library.Id}">
                    <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                      <path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z"/>
                      <polyline points="14 2 14 8 20 8"/>
                    </svg>
                    <span>Rescan ComicInfo.xml</span>
                  </button>
                </div>
              </div>
              </div>
              <div class="library-progress" aria-hidden="true"></div>
            </div>
            {else Library}
            <div class="empty-state">No libraries yet. Add one in YACReaderLibrary on this computer, or run <code>YACReaderLibraryServer add-library &lt;name&gt; &lt;path&gt;</code> on a headless server.</div>
            {end Library}
          </div>
        </section>

        <section class="section" aria-labelledby="server-heading">
          <div class="section-title" id="server-heading">Server</div>
          <div class="system-grid">
            <div class="meta-card">
              <span class="meta-label">Address</span>
              <span class="meta-value" title="{connect.address}">{connect.address}</span>
            </div>
            <div class="meta-card">
              <span class="meta-label">YACReader</span>
              <span class="meta-value">{yr.version}</span>
            </div>
            <div class="meta-card">
              <span class="meta-label">Server</span>
              <span class="meta-value">{server.version}</span>
            </div>
            <div class="meta-card">
              <span class="meta-label">Operating system</span>
              <span class="meta-value os">{os.name}</span>
            </div>
          </div>
        </section>
      </div>
    </main>
  </div>
  {if connect.available}
  <template id="connect-dialog-template">
    <div class="mobile-promotion-overlay">
      <section class="mobile-promotion-dialog connect-dialog" role="dialog" aria-modal="true" aria-labelledby="connect-dialog-title" aria-describedby="connect-dialog-copy">
        <button class="mobile-promotion-close" type="button" data-connect-close aria-label="Close">
          <svg class="mobile-promotion-close-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><path d="m7 7 10 10M17 7 7 17"/></svg>
        </button>
        <header class="mobile-promotion-intro">
          <h2 id="connect-dialog-title">Connect a device</h2>
          <p id="connect-dialog-copy">In YACReader for iOS or Android, add a new server and scan this code, or type the address. Your device needs to be on the same network as this server.</p>
        </header>
        <div class="connect-body">
          <div class="connect-qr">
            {loop Address}
            <div class="connect-qr-code" data-connect-qr="{Address.Value}" hidden>{Address.Svg}</div>
            {end Address}
          </div>
          <div class="connect-details">
            <span class="meta-label">Server address</span>
            <span class="meta-value connect-address" data-connect-address></span>
            <label class="connect-picker" data-connect-picker hidden>
              <span class="connect-picker-hint">This computer has more than one network address. Pick the one on your device's network.</span>
              <select data-connect-select aria-label="Network address">
                {loop Address}
                <option value="{Address.Value}">{Address.Value}</option>
                {end Address}
              </select>
            </label>
            <div class="connect-apps">
              <span class="meta-label">Don't have the app yet?</span>
              <div class="connect-store-links">
                <a class="store-badge-link app-store-badge" href="https://apps.apple.com/app/id635717885" target="_blank" rel="noopener noreferrer" aria-label="Download YACReader on the App Store"><img src="/images/webui/app-store-badge.svg" alt=""></a>
                <a class="store-badge-link google-play-badge" href="https://play.google.com/store/apps/details?id=com.yacreader.yacreader" target="_blank" rel="noopener noreferrer" aria-label="Get YACReader on Google Play"><img src="/images/webui/google-play-badge.png" alt=""></a>
              </div>
            </div>
          </div>
        </div>
      </section>
    </div>
  </template>
  {end connect.available}
</body>
</html>
)HTML"),
            "StatusPage");

    statusPage.enableWarnings();
    statusPage.setVariable("os.name", QSysInfo::prettyProductName().toHtmlEscaped());
    statusPage.setVariable("os.port", requestPort(request));
    statusPage.setVariable("server.version", SERVER_VERSION_NUMBER);
    statusPage.setVariable("yr.version", VERSION);

    const QStringList addresses = connectAddresses(request);
    statusPage.setCondition("connect.available", !addresses.isEmpty());
    statusPage.setVariable("connect.address", addresses.isEmpty() ? QString(QChar(0x2014)) : addresses.first().toHtmlEscaped());
    statusPage.loop("Address", addresses.size());
    for (int i = 0; i < addresses.size(); i++) {
        statusPage.setVariable(QString("Address%1.Value").arg(i), addresses.at(i).toHtmlEscaped());
        statusPage.setVariable(QString("Address%1.Svg").arg(i), qrCodeSvg(addresses.at(i)));
    }

    YACReaderLibraries libraries = DBHelper::getLibraries();
    const QList<QString> libraryNames = libraries.getNames();
    const int libraryCount = libraryNames.size();

    statusPage.setVariable("library.count", QString::number(libraryCount));
    statusPage.loop("Library", libraryCount);
    for (int i = 0; i < libraryCount; i++) {
        const QString libraryName = libraryNames.at(i);
        const QString libraryInitial = libraryName.trimmed().isEmpty() ? QString(QChar(0x2014)) : libraryName.trimmed().left(1).toUpper();

        statusPage.setVariable(QString("Library%1.Id").arg(i), QString::number(libraries.getId(libraryName)));
        statusPage.setVariable(QString("Library%1.Initial").arg(i), libraryInitial.toHtmlEscaped());
        statusPage.setVariable(QString("Library%1.Name").arg(i), libraryName.toHtmlEscaped());
        statusPage.setVariable(QString("Library%1.Path").arg(i), libraries.getPath(libraryName).toHtmlEscaped());
    }

    response.write(statusPage.toUtf8(), true);
}

void WebUIController::renderLibraryBrowser(HttpRequest &request,
                                           HttpResponse &response,
                                           int libraryId,
                                           const QString &libraryName,
                                           const QString &initialView,
                                           qulonglong initialItemId)
{
    setPageHeaders(response);

    Template browserPage(
            QStringLiteral(R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <meta name="color-scheme" content="light dark">
  <title>{library.name} · YACReaderLibrary</title>
  <link rel="stylesheet" href="/css/webui.css">
  <script src="/js/webui.js"></script>
</head>
<body data-browser-library-id="{library.id}" data-browser-library-name="{library.name}" data-browser-initial-view="{browser.view}" data-browser-initial-item-id="{browser.item.id}">
  <div class="app-shell">
    <aside class="sidebar">
      <div class="sidebar-header">
        <a class="brand" href="/webui">
          <img class="brand-logo" src="/images/webui/YACLibraryServer.svg" alt="">
          <span class="brand-copy">
            <span class="brand-name">YACReader</span>
            <span class="brand-product">Library Server</span>
          </span>
        </a>
        <button class="theme-toggle" data-theme-toggle type="button" aria-label="Use dark theme">
          <svg class="moon" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
            <path d="M21 12.8A9 9 0 1 1 11.2 3 7 7 0 0 0 21 12.8z"/>
          </svg>
          <svg class="sun" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
            <circle cx="12" cy="12" r="4"/>
            <path d="M12 2v2M12 20v2M4.93 4.93l1.42 1.42M17.66 17.66l1.41 1.41M2 12h2M20 12h2M4.93 19.07l1.42-1.42M17.66 6.34l1.41-1.41"/>
          </svg>
        </button>
      </div>

      <nav class="navigation" aria-label="Server">
        <a class="navigation-item" href="/webui">
          <svg width="18" height="18" viewBox="0 0 18 18" fill="none" aria-hidden="true">
            <circle cx="9" cy="9" r="6.4" stroke="currentColor" stroke-width="1.6"/>
            <circle cx="9" cy="9" r="2.6" fill="currentColor"/>
          </svg>
          Home
        </a>
        <a class="navigation-item active" href="/webui/library/{library.id}" aria-current="page">
          <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
            <path d="M4 5.5A2.5 2.5 0 0 1 6.5 3H10l2 2h5.5A2.5 2.5 0 0 1 20 7.5v9A2.5 2.5 0 0 1 17.5 19h-11A2.5 2.5 0 0 1 4 16.5z"/>
          </svg>
          Library
        </a>
        <a class="navigation-item" href="/webui/settings">
          <svg width="18" height="18" viewBox="0 0 18 18" fill="currentColor" aria-hidden="true">
            <rect x="2.4" y="4.1" width="13.2" height="1.6" rx=".8"/>
            <circle cx="12" cy="4.9" r="2.4"/>
            <rect x="2.4" y="11.4" width="13.2" height="1.6" rx=".8"/>
            <circle cx="6" cy="12.2" r="2.4"/>
          </svg>
          Settings
        </a>
      </nav>

      <div class="server-summary">
        <span class="online-dot" aria-hidden="true"></span>
        <span>Server {server.version} · :{os.port}</span>
      </div>
    </aside>

    <main class="main">
      <header class="topbar browser-topbar">
        <button class="browser-back" data-browser-back type="button" aria-label="Go to containing folder" hidden>
          <svg width="19" height="19" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
            <path d="m15 18-6-6 6-6"/>
          </svg>
        </button>
        <div class="browser-heading">
          <nav class="breadcrumbs" data-browser-breadcrumbs aria-label="Breadcrumb">
            <a href="/webui#libraries">Libraries</a>
            <span aria-hidden="true">/</span>
            <span>{library.name}</span>
          </nav>
          <h1 class="page-title" data-browser-title>{library.name}</h1>
        </div>
        <form class="browser-search" data-browser-search role="search" hidden>
          <button class="browser-search-submit" type="submit" aria-label="Search this library">
            <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
              <circle cx="11" cy="11" r="7"/>
              <path d="m20 20-4-4"/>
            </svg>
          </button>
          <label class="visually-hidden" for="library-search">Search this library</label>
          <input id="library-search" data-browser-search-input type="search" inputmode="search" autocomplete="off" spellcheck="false" placeholder="Search or use field:value">
          <button class="browser-search-clear" data-browser-search-clear type="button" aria-label="Clear search" hidden>
            <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" aria-hidden="true">
              <path d="m7 7 10 10M17 7 7 17"/>
            </svg>
          </button>
        </form>
      </header>

      <div class="content browser-content" data-browser-root aria-live="polite">
        <div class="browser-loading">
          <div class="browser-loading-header"></div>
          <div class="browser-loading-grid">
            <div class="browser-loading-card"></div>
            <div class="browser-loading-card"></div>
            <div class="browser-loading-card"></div>
            <div class="browser-loading-card"></div>
          </div>
        </div>
      </div>
    </main>
  </div>
</body>
</html>
)HTML"),
            "LibraryBrowserPage");

    browserPage.enableWarnings();
    browserPage.setVariable("library.id", QString::number(libraryId));
    browserPage.setVariable("library.name", libraryName.toHtmlEscaped());
    browserPage.setVariable("browser.view", initialView.toHtmlEscaped());
    browserPage.setVariable("browser.item.id", QString::number(initialItemId));
    browserPage.setVariable("os.port", requestPort(request));
    browserPage.setVariable("server.version", SERVER_VERSION_NUMBER);

    response.write(browserPage.toUtf8(), true);
}

void WebUIController::renderSettingsPage(HttpRequest &request, HttpResponse &response, const QString &errorMessage)
{
    setPageHeaders(response);

    QSettings settings(settingsFilePath(), QSettings::IniFormat);
    settings.beginGroup("libraryConfig");
    settings.sync();

    const bool importComicInfo = settings.value(IMPORT_COMIC_INFO_XML_METADATA, false).toBool();
    const bool updateAtStartup = settings.value(UPDATE_LIBRARIES_AT_STARTUP, false).toBool();
    const bool updatePeriodically = settings.value(UPDATE_LIBRARIES_PERIODICALLY, false).toBool();
    int updateInterval = settings.value(UPDATE_LIBRARIES_PERIODICALLY_INTERVAL, static_cast<int>(YACReader::LibrariesUpdateInterval::Hours2)).toInt();
    const bool updateAtCertainTime = settings.value(UPDATE_LIBRARIES_AT_CERTAIN_TIME, false).toBool();
    QTime updateTime = settings.value(UPDATE_LIBRARIES_AT_CERTAIN_TIME_TIME, QStringLiteral("00:00")).toTime();

    if (updateInterval < static_cast<int>(YACReader::LibrariesUpdateInterval::Minutes30) || updateInterval > static_cast<int>(YACReader::LibrariesUpdateInterval::Daily)) {
        updateInterval = static_cast<int>(YACReader::LibrariesUpdateInterval::Hours2);
    }

    if (!updateTime.isValid()) {
        updateTime = QTime(0, 0);
    }

    Template settingsPage(
            QStringLiteral(R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <meta name="color-scheme" content="light dark">
  <title>Settings · YACReaderLibrary</title>
  <link rel="stylesheet" href="/css/webui.css">
  <script src="/js/webui.js"></script>
</head>
<body>
  <div class="app-shell">
    <aside class="sidebar">
      <div class="sidebar-header">
        <a class="brand" href="/webui">
          <img class="brand-logo" src="/images/webui/YACLibraryServer.svg" alt="">
          <span class="brand-copy">
            <span class="brand-name">YACReader</span>
            <span class="brand-product">Library Server</span>
          </span>
        </a>
        <button class="theme-toggle" data-theme-toggle type="button" aria-label="Use dark theme">
          <svg class="moon" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
            <path d="M21 12.8A9 9 0 1 1 11.2 3 7 7 0 0 0 21 12.8z"/>
          </svg>
          <svg class="sun" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
            <circle cx="12" cy="12" r="4"/>
            <path d="M12 2v2M12 20v2M4.93 4.93l1.42 1.42M17.66 17.66l1.41 1.41M2 12h2M20 12h2M4.93 19.07l1.42-1.42M17.66 6.34l1.41-1.41"/>
          </svg>
        </button>
      </div>

      <nav class="navigation" aria-label="Server">
        <a class="navigation-item" href="/webui">
          <svg width="18" height="18" viewBox="0 0 18 18" fill="none" aria-hidden="true">
            <circle cx="9" cy="9" r="6.4" stroke="currentColor" stroke-width="1.6"/>
            <circle cx="9" cy="9" r="2.6" fill="currentColor"/>
          </svg>
          Home
        </a>
        <a class="navigation-item active" href="/webui/settings" aria-current="page">
          <svg width="18" height="18" viewBox="0 0 18 18" fill="currentColor" aria-hidden="true">
            <rect x="2.4" y="4.1" width="13.2" height="1.6" rx=".8"/>
            <circle cx="12" cy="4.9" r="2.4"/>
            <rect x="2.4" y="11.4" width="13.2" height="1.6" rx=".8"/>
            <circle cx="6" cy="12.2" r="2.4"/>
          </svg>
          Settings
        </a>
      </nav>

      <div class="server-summary">
        <span class="online-dot" aria-hidden="true"></span>
        <span>Server {server.version} · :{os.port}</span>
      </div>
    </aside>

    <main class="main">
      <header class="topbar">
        <div>
          <div class="eyebrow">Configuration</div>
          <h1 class="page-title">Settings</h1>
        </div>
      </header>

      <div class="content settings-content">
        <div class="settings-intro">
          <div>
            <div class="section-title">Advanced settings</div>
            <h2>Library automation</h2>
            <p>Control automatic library scans and metadata import.</p>
          </div>
        </div>

        {if saved}
        <div class="notice success" role="status">
          <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
            <path d="M20 6 9 17l-5-5"/>
          </svg>
          Settings saved.
        </div>
        {end saved}

        {if error}
        <div class="notice error" role="alert">
          <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
            <circle cx="12" cy="12" r="9"/>
            <path d="M12 8v5M12 16h.01"/>
          </svg>
          {settings.error}
        </div>
        {end error}

        <form class="settings-form" method="post" action="/webui/settings">
          <input type="hidden" name="csrfToken" value="{csrf.token}">

          <section class="settings-card" aria-labelledby="metadata-heading">
            <div class="settings-card-heading">
              <div>
                <div class="section-title">Metadata</div>
                <h3 id="metadata-heading">ComicInfo.xml legacy support</h3>
              </div>
            </div>
            <label class="setting-row" for="importComicInfo">
              <span class="setting-copy">
                <span class="setting-name">Import ComicInfo.xml metadata</span>
                <span class="setting-description">Parse legacy XML metadata when new comics are added to a library.</span>
              </span>
              <span class="switch">
                <input id="importComicInfo" name="importComicInfo" type="checkbox" {import.checked}>
                <span class="switch-track" aria-hidden="true"></span>
              </span>
            </label>
          </section>

          <section class="settings-card" aria-labelledby="updates-heading">
            <div class="settings-card-heading">
              <div>
                <div class="section-title">Libraries</div>
                <h3 id="updates-heading">Automatic updates</h3>
              </div>
              <span class="settings-badge">Server time · {server.time}</span>
            </div>

            <label class="setting-row" for="updateAtStartup">
              <span class="setting-copy">
                <span class="setting-name">Update at startup</span>
                <span class="setting-description">Scan all configured libraries when the server starts. This takes effect after the next restart.</span>
              </span>
              <span class="switch">
                <input id="updateAtStartup" name="updateAtStartup" type="checkbox" {startup.checked}>
                <span class="switch-track" aria-hidden="true"></span>
              </span>
            </label>

            <div class="setting-row setting-row-with-control">
              <label class="setting-copy" for="updatePeriodically">
                <span class="setting-name">Update periodically</span>
                <span class="setting-description">Rescan libraries after a recurring interval.</span>
              </label>
              <div class="setting-control-group">
                <select id="updateInterval" name="updateInterval" aria-label="Periodic update interval">
                  <option value="0" {interval.0.selected}>Every 30 minutes</option>
                  <option value="1" {interval.1.selected}>Every hour</option>
                  <option value="2" {interval.2.selected}>Every 2 hours</option>
                  <option value="3" {interval.3.selected}>Every 4 hours</option>
                  <option value="4" {interval.4.selected}>Every 8 hours</option>
                  <option value="5" {interval.5.selected}>Every 12 hours</option>
                  <option value="6" {interval.6.selected}>Daily</option>
                </select>
                <label class="switch" for="updatePeriodically">
                  <input id="updatePeriodically" name="updatePeriodically" type="checkbox" data-controls="updateInterval" {periodic.checked}>
                  <span class="switch-track" aria-hidden="true"></span>
                </label>
              </div>
            </div>

            <div class="setting-row setting-row-with-control">
              <label class="setting-copy" for="updateAtTime">
                <span class="setting-name">Update at a certain time</span>
                <span class="setting-description">Run a daily scan at the selected time using the server's local clock.</span>
              </label>
              <div class="setting-control-group">
                <input id="updateTime" name="updateTime" type="time" value="{scheduled.time}" aria-label="Scheduled update time">
                <label class="switch" for="updateAtTime">
                  <input id="updateAtTime" name="updateAtTime" type="checkbox" data-controls="updateTime" {scheduled.checked}>
                  <span class="switch-track" aria-hidden="true"></span>
                </label>
              </div>
            </div>

            <div class="warning-note">
              <svg width="19" height="19" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                <path d="M10.3 3.7 2.2 18a2 2 0 0 0 1.7 3h16.2a2 2 0 0 0 1.7-3L13.7 3.7a2 2 0 0 0-3.4 0z"/>
                <path d="M12 9v4M12 17h.01"/>
              </svg>
              <span>Database writes are disabled while a library update is running. Schedule scans for times when clients are unlikely to be active.</span>
            </div>
          </section>

          <div class="form-actions">
            <p>Periodic and scheduled changes are picked up by the running server within a minute.</p>
            <button class="primary-button" type="submit">Save changes</button>
          </div>
        </form>

        <section class="settings-card browser-settings-card" aria-labelledby="browser-heading">
          <div class="settings-card-heading">
            <div>
              <div class="section-title">This browser</div>
              <h3 id="browser-heading">Library browser</h3>
            </div>
          </div>
          <div class="setting-row setting-row-with-control">
            <label class="setting-copy" for="recentDays">
              <span class="setting-name">Days to consider a comic recent</span>
              <span class="setting-description">Recent shows the comics added in these days. This setting is saved in this browser only, and it changes at once.</span>
            </label>
            <div class="setting-control-group">
              <select id="recentDays" data-recent-days aria-label="Days to consider a comic recent"></select>
            </div>
          </div>
        </section>
      </div>
    </main>
  </div>
</body>
</html>
)HTML"),
            "SettingsPage");

    settingsPage.enableWarnings();
    settingsPage.setVariable("os.port", requestPort(request));
    settingsPage.setVariable("server.version", SERVER_VERSION_NUMBER);
    settingsPage.setVariable("server.time", QTime::currentTime().toString(QStringLiteral("HH:mm")));
    settingsPage.setVariable("csrf.token", QString::fromUtf8(settingsCsrfToken()).toHtmlEscaped());
    settingsPage.setVariable("import.checked", checkedAttribute(importComicInfo));
    settingsPage.setVariable("startup.checked", checkedAttribute(updateAtStartup));
    settingsPage.setVariable("periodic.checked", checkedAttribute(updatePeriodically));
    settingsPage.setVariable("scheduled.checked", checkedAttribute(updateAtCertainTime));
    settingsPage.setVariable("scheduled.time", updateTime.toString(QStringLiteral("HH:mm")));

    for (int i = static_cast<int>(YACReader::LibrariesUpdateInterval::Minutes30);
         i <= static_cast<int>(YACReader::LibrariesUpdateInterval::Daily);
         i++) {
        settingsPage.setVariable(QString("interval.%1.selected").arg(i), selectedAttribute(updateInterval, i));
    }

    settingsPage.setCondition("saved", request.getParameter("saved") == "1");
    settingsPage.setCondition("error", !errorMessage.isEmpty());
    if (!errorMessage.isEmpty()) {
        settingsPage.setVariable("settings.error", errorMessage.toHtmlEscaped());
    }

    response.write(settingsPage.toUtf8(), true);
}

bool WebUIController::saveSettings(HttpRequest &request, QString &errorMessage, int &errorStatus)
{
    if (request.getParameter("csrfToken") != settingsCsrfToken()) {
        errorMessage = QStringLiteral("The settings form expired. Reload the page and try again.");
        errorStatus = 403;
        return false;
    }

    QSettings settings(settingsFilePath(), QSettings::IniFormat);
    settings.beginGroup("libraryConfig");
    settings.sync();

    const auto parameters = request.getParameterMap();
    const bool updatePeriodically = parameters.contains("updatePeriodically");
    const bool updateAtTime = parameters.contains("updateAtTime");

    int updateInterval = settings.value(UPDATE_LIBRARIES_PERIODICALLY_INTERVAL, static_cast<int>(YACReader::LibrariesUpdateInterval::Hours2)).toInt();
    if (parameters.contains("updateInterval")) {
        bool intervalIsValid = false;
        const int submittedInterval = request.getParameter("updateInterval").toInt(&intervalIsValid);
        if (!intervalIsValid || submittedInterval < static_cast<int>(YACReader::LibrariesUpdateInterval::Minutes30) || submittedInterval > static_cast<int>(YACReader::LibrariesUpdateInterval::Daily)) {
            errorMessage = QStringLiteral("Choose a valid periodic update interval.");
            errorStatus = 400;
            return false;
        }
        updateInterval = submittedInterval;
    } else if (updatePeriodically) {
        errorMessage = QStringLiteral("Choose a periodic update interval.");
        errorStatus = 400;
        return false;
    }

    QTime updateTime = settings.value(UPDATE_LIBRARIES_AT_CERTAIN_TIME_TIME, QStringLiteral("00:00")).toTime();
    if (parameters.contains("updateTime")) {
        const QTime submittedTime = QTime::fromString(QString::fromUtf8(request.getParameter("updateTime")), QStringLiteral("HH:mm"));
        if (!submittedTime.isValid()) {
            errorMessage = QStringLiteral("Choose a valid scheduled update time.");
            errorStatus = 400;
            return false;
        }
        updateTime = submittedTime;
    } else if (updateAtTime) {
        errorMessage = QStringLiteral("Choose a scheduled update time.");
        errorStatus = 400;
        return false;
    }

    if (!updateTime.isValid()) {
        updateTime = QTime(0, 0);
    }

    settings.setValue(IMPORT_COMIC_INFO_XML_METADATA, parameters.contains("importComicInfo"));
    settings.setValue(UPDATE_LIBRARIES_AT_STARTUP, parameters.contains("updateAtStartup"));
    settings.setValue(UPDATE_LIBRARIES_PERIODICALLY, updatePeriodically);
    settings.setValue(UPDATE_LIBRARIES_PERIODICALLY_INTERVAL, updateInterval);
    settings.setValue(UPDATE_LIBRARIES_AT_CERTAIN_TIME, updateAtTime);
    settings.setValue(UPDATE_LIBRARIES_AT_CERTAIN_TIME_TIME, updateTime.toString(QStringLiteral("HH:mm")));
    settings.sync();

    if (settings.status() != QSettings::NoError) {
        errorMessage = QStringLiteral("YACReader could not write the settings file. Check its permissions and try again.");
        errorStatus = 500;
        return false;
    }

    return true;
}
