#include "statuspagecontroller.h"

#include "template.h"
#include "yacreader_global.h"
#include "db_helper.h"
#include "yacreader_libraries.h"
#include "QsLog.h"

#include <QSysInfo>

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;
// using stefanfrings::HttpSession;
using stefanfrings::Template;

StatusPageController::StatusPageController() { }

void StatusPageController::service(HttpRequest &request, HttpResponse &response)
{
    response.setHeader("Content-Type", "text/html; charset=utf-8");
    response.setHeader("Connection", "close");

    Template StatusPage = Template(
            QStringLiteral(
                    "<!DOCTYPE html>\n"
                    "<html>\n"
                    "<head>\n"
                    "<title>YACReaderLibrary Server</title>\n"
                    "</head>\n"
                    "<body>\n\n"
                    "<center>\n"
                    "<img src='/images/webui/YACLibraryServer.svg' width=15%>\n"
                    "<h1>YACReaderLibraryServer is up and running.</h1>\n"
                    "<p>YACReader {yr.version}<p>\n"
                    "<p>Server {server.version}<p>\n"
                    "<p>OS:\t{os.name} {os.version}</p>\n"
                    "<p>Port:\t{os.port}</p>\n"
                    "<table>\n"
                    "<thead>\n"
                    "<tr>\n"
                    "<th>Library</th>\n"
                    "<th>Path</th>\n"
                    "</tr>\n"
                    "</thead>\n"
                    "{loop Library}"
                    "<tr>\n"
                    "<td>{Library.Name}</td>\n"
                    "<td>{Library.Path}</td>\n"
                    "<tr>\n"
                    "{end Library}"
                    "</p>\n"
                    "</center>\n"
                    "</body>\n"
                    "</html>\n"),

            "StatusPage");

    StatusPage.enableWarnings();

    // Set template variables
    StatusPage.setVariable("os.name", QSysInfo::prettyProductName());
    StatusPage.setVariable("os.version", QSysInfo::productVersion());
    // Getting the port from the request is basically a hack, but should do the trick
    StatusPage.setVariable("os.port", QString(request.getHeader("host")).split(":")[1]);

    StatusPage.setVariable("server.version", SERVER_VERSION_NUMBER);
    StatusPage.setVariable("yr.version", VERSION);

    // Get library info
    YACReaderLibraries libraries = DBHelper::getLibraries();
    QList<QString> library_names = libraries.getNames();
    size_t num_libs = libraries.getNames().size();

    // Fill template
    StatusPage.loop("Library", num_libs);
    for (size_t i = 0; i < num_libs; i++) {
        StatusPage.setVariable(QString("Library%1.Name").arg(i), library_names.at(i));
        StatusPage.setVariable(QString("Library%1.Path").arg(i), libraries.getPath(library_names.at(i)));
    }

    response.write(StatusPage.toUtf8(), true);
}
