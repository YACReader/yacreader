#include "webuicontroller.h"

#include "db_helper.h"
#include "template.h"
#include "yacreader_global.h"
#include "yacreader_libraries.h"

#include <QCoreApplication>
#include <QDir>
#include <QRegularExpression>
#include <QSettings>
#include <QSysInfo>
#include <QTime>
#include <QUrl>
#include <QUuid>

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
    response.setHeader("Content-Security-Policy", "default-src 'self'; img-src 'self'; style-src 'self'; script-src 'self'; base-uri 'none'; form-action 'self'; frame-ancestors 'none'");
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
            QStringLiteral(R"(^/webui/library/([0-9]+)(?:/(folder|comic)/([0-9]+))?/?$)"));
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

        const QString initialView = libraryBrowserMatch.captured(2).isEmpty() ? QStringLiteral("folder") : libraryBrowserMatch.captured(2);
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
  <title>Server status · YACReaderLibrary</title>
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
          Status
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
          <div class="eyebrow">Overview</div>
          <h1 class="page-title">Server status</h1>
        </div>
      </header>

      <div class="content">
        <section class="status-card">
          <div class="status-copy">
            <div class="status-pill">
              <span class="online-dot" aria-hidden="true"></span>
              Online
            </div>
            <h2 class="status-heading">YACReaderLibrary Server is up and running.</h2>
            <p class="status-description">Your libraries are mounted and ready for YACReader clients on your network.</p>
          </div>
          <img class="hero-logo" src="/images/webui/YACLibraryServer.svg" alt="YACReader">
        </section>

        <section class="section" aria-labelledby="system-heading">
          <div class="section-title" id="system-heading">System</div>
          <div class="system-grid">
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
            <div class="meta-card">
              <span class="meta-label">Port</span>
              <span class="meta-value">{os.port}</span>
            </div>
          </div>
        </section>

        <section class="section" id="libraries" aria-labelledby="libraries-heading">
          <div class="section-header">
            <div class="section-title" id="libraries-heading">Libraries</div>
            <span class="count">{library.count}</span>
          </div>
          <p class="library-description">Libraries currently available to connected YACReader clients.</p>
          <div class="library-grid">
            {loop Library}
            <a class="library-card" href="/webui/library/{Library.Id}" aria-label="Browse {Library.Name}">
              <div class="library-initial" aria-hidden="true">{Library.Initial}</div>
              <div class="library-copy">
                <div class="library-name" title="{Library.Name}">{Library.Name}</div>
                <div class="library-path" title="{Library.Path}">{Library.Path}</div>
              </div>
              <svg class="library-arrow" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                <path d="m9 18 6-6-6-6"/>
              </svg>
            </a>
            {else Library}
            <div class="empty-state">No libraries are configured yet.</div>
            {end Library}
          </div>
        </section>
      </div>
    </main>
  </div>
</body>
</html>
)HTML"),
            "StatusPage");

    statusPage.enableWarnings();
    statusPage.setVariable("os.name", QSysInfo::prettyProductName().toHtmlEscaped());
    statusPage.setVariable("os.port", requestPort(request));
    statusPage.setVariable("server.version", SERVER_VERSION_NUMBER);
    statusPage.setVariable("yr.version", VERSION);

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
          Status
        </a>
        <a class="navigation-item active" href="/webui#libraries" aria-current="page">
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
          Status
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
