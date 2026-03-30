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


bool database::takeBook(int userId, int bookId)
{
    QSqlDatabase conn = database::db();
    QSqlQuery q(conn);

    conn.transaction();

    q.prepare("SELECT status FROM Books WHERE id = :book_id");
    q.bindValue(":bookId", bookId);
    if(!q.exec() || !q.next()){
        conn.rollback();
        return false;
    }
    if(q.value(0).toString() != "available"){
        conn.rollback();
        return false;
    }

    q.prepare("INSERT INTO Loans (user_id, book_id, issue_date, due_date, status) "
              "VALUES (:userId, :bookId, DATE('now'), DATE('now', '+14 day'), 'active')");
    q.bindValue(":userId", userId);
    q.bindValue(":bookId", bookId);
    if(!q.exec()){
        conn.rollback();
        return false;
    }

    q.prepare("UPDATE Books SET status = 'taken' WHERE id = :bookId");
    q.bindValue(":bookId", bookId);
    if (!q.exec()) {
        conn.rollback();
        return false;
    }

    conn.commit();
    return true;
}


