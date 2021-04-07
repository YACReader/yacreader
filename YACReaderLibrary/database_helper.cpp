#include "database_helper.h"

#include "data_base_management.h"

#include "QsLog.h"

using namespace YACReader;

DatabaseHolder::DatabaseHolder(const QString &path)
    : DatabaseHolder { DataBaseManagement::loadDatabase(path) }
{
}

DatabaseHolder::DatabaseHolder(const QSqlDatabase &db)
    : db { db }
{
    remover.setDb(this->db);
}

void DatabaseHolder::Remover::setDb(const QSqlDatabase &db)
{
    connectionName = db.connectionName();
    Q_ASSERT(!connectionName.isEmpty());
}

DatabaseHolder::Remover::~Remover()
{
    if (connectionName.isEmpty()) {
        QLOG_WARN() << "DatabaseHolder::Remover cannot remove a database: empty connection name.";
        return;
    }
    QSqlDatabase::removeDatabase(connectionName);
}
