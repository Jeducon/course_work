#include "database.h"
#include <QtSql/QSqlQuery>

QSqlDatabase database::m_db;

bool database::init(const QString &path)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(path);
    qDebug() << "DB path =" << database::db().databaseName();

    if (!m_db.open())
        return false;

    QSqlQuery q(m_db);
    q.exec("CREATE TABLE IF NOT EXISTS Users ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "username TEXT UNIQUE NOT NULL,"
           "password TEXT NOT NULL,"
           "role TEXT NOT NULL,"
           "full_name TEXT,"
           "address TEXT,"
           "phone TEXT,"
           "email TEXT,"
           "photo_path TEXT"
           ")");
    q.exec("INSERT OR IGNORE INTO Users(username, password, role, full_name, address, phone, "
           "email, photo_path) "
           "VALUES('admin', 'admin', 'admin', 'Admin Admin', 'Admin street', '0000000000', "
           "'admin@example.com', '')");
    q.exec("CREATE TABLE IF NOT EXISTS Books ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "title TEXT NOT NULL UNIQUE,"
           "author TEXT NOT NULL,"
           "genre TEXT NOT NULL,"
           "year INTEGER NOT NULL CHECK(year > 0),"
           "status TEXT NOT NULL,"
           "cover_path TEXT"
           ")");
    q.exec("CREATE TABLE IF NOT EXISTS Loans("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "user_id INTEGER NOT NULL,"
           "book_id INTEGER NOT NULL,"
           "issue_date TEXT NOT NULL,"
           "due_date TEXT,"
           "return_date TEXT,"
           "status TEXT NOT NULL"
           ")");
    return true;
}

QSqlDatabase database::db()
{
    return m_db;
}
