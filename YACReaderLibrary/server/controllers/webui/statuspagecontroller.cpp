#include "statuspagecontroller.h"

#include "template.h"
#include "yacreader_global.h"
#include "db_helper.h"
#include "yacreader_libraries.h"

#include <QSysInfo>

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;
using stefanfrings::Template;

StatusPageController::StatusPageController() { }

void StatusPageController::service(HttpRequest &request, HttpResponse &response)
{
    response.setHeader("Content-Type", "text/html; charset=utf-8");
    response.setHeader("Connection", "close");

    Template StatusPage = Template(
            QStringLiteral(
                    "<!DOCTYPE html>\n"
                    "<html lang='en'>\n"
                    "<head>\n"
                    "  <meta charset='UTF-8'>\n"
                    "  <meta name='viewport' content='width=device-width, initial-scale=1.0'>\n"
                    "  <link href='https://fonts.googleapis.com/css?family=Open+Sans:300,400,700,300italic,400italic,800italic' rel='stylesheet' type='text/css'>"
                    "  <title>YACReaderLibrary Server</title>\n"
                    "  <style>\n"
                    "    body {\n"
                    "      font-family: 'Open Sans', sans-serif;\n"
                    "      margin: 0;\n"
                    "      padding: 0;\n"
                    "    }\n"
                    "    header {\n"
                    "      text-align: center;\n"
                    "      padding: 20px;\n"
                    "      padding-top: 80px;\n"
                    "      background-color: #fff;\n"
                    "      color: rgb(34, 34, 34);\n"
                    "    }\n"
                    "    header h1 {\n"
                    "      font-size: 40px;\n"
                    "      font-style: normal;\n"
                    "      font-weight: 400;\n"
                    "    }\n"
                    "    img {\n"
                    "      max-width: 100%;\n"
                    "      height: auto;\n"
                    "    }\n"
                    "    h1 {\n"
                    "      color: rgb(34, 34, 34);\n"
                    "    }\n"
                    "    section {\n"
                    "      margin: 20px;\n"
                    "    }\n"
                    "    table {\n"
                    "      border-collapse: collapse;\n"
                    "      margin: 20px auto;\n"
                    "    }\n"
                    "    th, td {\n"
                    "      border: 1px solid #ddd;\n"
                    "      padding: 8px;\n"
                    "      text-align: left;\n"
                    "    }\n"
                    "    th {\n"
                    "      background-color: #eee;\n"
                    "      color: rgb(34, 34, 34);\n"
                    "    }\n"
                    "  </style>\n"
                    "</head>\n"
                    "<body>\n"
                    "  <header>\n"
                    "    <img src='/images/webui/YACLibraryServer.svg' alt='YACReaderLibrary Server Logo' width='200px'>\n"
                    "    <h1>YACReaderLibrary Server <span style = 'display: block;'>is up and running.</span></h1>"
                    "    <p>YACReader {yr.version}</p>\n"
                    "    <p>Server {server.version}</p>\n"
                    "    <p>OS: {os.name} {os.version}</p>\n"
                    "    <p>Port: {os.port}</p>\n"
                    "  </header>\n"
                    "  <section>\n"
                    "    <table>\n"
                    "      <thead>\n"
                    "        <tr>\n"
                    "          <th>Library</th>\n"
                    "          <th>Path</th>\n"
                    "        </tr>\n"
                    "      </thead>\n"
                    "      <tbody>\n"
                    "        {loop Library}\n"
                    "          <tr>\n"
                    "            <td>{Library.Name}</td>\n"
                    "            <td>{Library.Path}</td>\n"
                    "          </tr>\n"
                    "        {end Library}\n"
                    "      </tbody>\n"
                    "    </table>\n"
                    "  </section>\n"
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
