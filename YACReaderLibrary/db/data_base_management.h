#ifndef __DATA_BASE_MANAGEMENT_H
#define __DATA_BASE_MANAGEMENT_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QString>
#include <QStringList>
#include <QThread>

class ComicsInfoExporter : public QThread
{
    Q_OBJECT
public:
    ComicsInfoExporter();
    void exportComicsInfo(QSqlDatabase &source, QSqlDatabase &dest);

private:
    void run() override;
};

class ComicsInfoImporter : public QThread
{
    Q_OBJECT
public:
    ComicsInfoImporter();
    void importComicsInfo(QSqlDatabase &source, QSqlDatabase &dest);

private:
    void run() override;
};

class DataBaseManagement
{
private:
    static void bindString(const QString &name, const QSqlRecord &record, QSqlQuery &query);
    static void bindInt(const QString &name, const QSqlRecord &record, QSqlQuery &query);
    static void bindDouble(const QString &name, const QSqlRecord &record, QSqlQuery &query);
    static void bindValuesFromRecord(const QSqlRecord &record, QSqlQuery &query);

    static bool addColumns(const QString &tableName, const QStringList &columnDefs, const QSqlDatabase &db);
    static bool addConstraint(const QString &tableName, const QString &constraint, const QSqlDatabase &db);

public:
    //crea una base de datos y todas sus tablas
    static QSqlDatabase createDatabase(const QString &name, const QString &path);
    static QSqlDatabase createDatabase(const QString &dest);
    //carga una base de datos desde la ruta path
    static QSqlDatabase loadDatabase(const QString &path);
    static QSqlDatabase loadDatabaseFromFile(const QString &path);
    static bool createTables(QSqlDatabase &database);
    static bool createV8Tables(QSqlDatabase &database);

    static void exportComicsInfo(const QString &source, const QString &dest);
    static void importComicsInfo(const QString &source, const QString &dest);

    static QString checkValidDB(const QString &fullPath); //retorna "" si la DB es inválida ó la versión si es válida.
    static int compareVersions(const QString &v1, const QString v2); //retorna <0 si v1 < v2, 0 si v1 = v2 y >0 si v1 > v2
    static bool updateToCurrentVersion(const QString &path);
};

#endif
