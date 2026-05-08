#include "database.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

QSqlDatabase database::m_db;

static bool columnExists(QSqlDatabase db, const QString &tableName, const QString &columnName)
{
    QSqlQuery q(db);
    if (!q.exec(QString("PRAGMA table_info(%1)").arg(tableName)))
        return false;

    while (q.next()) {
        if (q.value(1).toString() == columnName)
            return true;
    }
    return false;
}

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

    q.exec("CREATE TABLE IF NOT EXISTS Authors ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "name TEXT NOT NULL UNIQUE COLLATE NOCASE"
           ")");

    q.exec("CREATE TABLE IF NOT EXISTS Books ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "title TEXT NOT NULL UNIQUE,"
           "author TEXT NOT NULL,"
           "genre TEXT NOT NULL,"
           "year INTEGER NOT NULL CHECK(year > 0),"
           "status TEXT NOT NULL,"
           "description TEXT,"
           "cover_path TEXT"
           ")");

    if (!columnExists(m_db, "Books", "description")) {
        q.exec("ALTER TABLE Books ADD COLUMN description TEXT");
    }

    if (columnExists(m_db, "Books", "author") && !columnExists(m_db, "Books", "author_id")) {
        if (!migrateBooksAuthorToAuthors()) {
            qDebug() << "Author migration failed";
            return false;
        }
    }

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

bool database::migrateBooksAuthorToAuthors()
{
    QSqlDatabase conn = database::db();
    if (!conn.transaction()) {
        qDebug() << "Migration transaction start failed:" << conn.lastError().text();
        return false;
    }

    QSqlQuery q(conn);

    if (!q.exec("ALTER TABLE Books RENAME TO Books_old")) {
        qDebug() << "Rename Books failed:" << q.lastError().text();
        conn.rollback();
        return false;
    }

    if (!q.exec("CREATE TABLE Books ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "title TEXT NOT NULL UNIQUE,"
                "author_id INTEGER NOT NULL,"
                "genre TEXT NOT NULL,"
                "year INTEGER NOT NULL CHECK(year > 0),"
                "status TEXT NOT NULL,"
                "description TEXT,"
                "cover_path TEXT,"
                "FOREIGN KEY(author_id) REFERENCES Authors(id)"
                ")")) {
        qDebug() << "Create new Books failed:" << q.lastError().text();
        conn.rollback();
        return false;
    }

    QSqlQuery selectOld(conn);
    if (!selectOld.exec("SELECT id, title, author, genre, year, status, description, cover_path FROM Books_old")) {
        qDebug() << "Select old books failed:" << selectOld.lastError().text();
        conn.rollback();
        return false;
    }

    while (selectOld.next()) {
        const int oldId = selectOld.value(0).toInt();
        const QString title = selectOld.value(1).toString();
        const QString authorName = selectOld.value(2).toString().trimmed();
        const QString genre = selectOld.value(3).toString();
        const int year = selectOld.value(4).toInt();
        const QString status = selectOld.value(5).toString();
        const QString description = selectOld.value(6).toString();
        const QString coverPath = selectOld.value(7).toString();

        const int authorId = ensureAuthorExists(authorName);
        if (authorId <= 0) {
            qDebug() << "Failed to ensure author:" << authorName;
            conn.rollback();
            return false;
        }

        QSqlQuery insertBook(conn);
        insertBook.prepare("INSERT INTO Books(id, title, author_id, genre, year, status, description, cover_path) "
                           "VALUES(:id, :title, :author_id, :genre, :year, :status, :description, :cover_path)");
        insertBook.bindValue(":id", oldId);
        insertBook.bindValue(":title", title);
        insertBook.bindValue(":author_id", authorId);
        insertBook.bindValue(":genre", genre);
        insertBook.bindValue(":year", year);
        insertBook.bindValue(":status", status);
        insertBook.bindValue(":description", description);
        insertBook.bindValue(":cover_path", coverPath);

        if (!insertBook.exec()) {
            qDebug() << "Insert migrated book failed:" << insertBook.lastError().text();
            conn.rollback();
            return false;
        }
    }

    if (!q.exec("DROP TABLE Books_old")) {
        qDebug() << "Drop old Books failed:" << q.lastError().text();
        conn.rollback();
        return false;
    }

    if (!conn.commit()) {
        qDebug() << "Migration commit failed:" << conn.lastError().text();
        conn.rollback();
        return false;
    }

    return true;
}

QSqlDatabase database::db()
{
    return m_db;
}

int database::ensureAuthorExists(const QString &authorName)
{
    const QString normalized = authorName.trimmed();
    if (normalized.isEmpty())
        return -1;

    QSqlQuery q(m_db);
    q.prepare("SELECT id FROM Authors WHERE name = :name COLLATE NOCASE");
    q.bindValue(":name", normalized);

    if (q.exec() && q.next())
        return q.value(0).toInt();

    QSqlQuery ins(m_db);
    ins.prepare("INSERT INTO Authors(name) VALUES(:name)");
    ins.bindValue(":name", normalized);

    if (!ins.exec()) {
        qDebug() << "Insert author failed:" << ins.lastError().text();
        return -1;
    }

    return ins.lastInsertId().toInt();
}

QString database::authorNameById(int authorId)
{
    QSqlQuery q(m_db);
    q.prepare("SELECT name FROM Authors WHERE id = :id");
    q.bindValue(":id", authorId);

    if (q.exec() && q.next())
        return q.value(0).toString();

    return QString();
}

QStringList database::allAuthorNames()
{
    QStringList result;
    QSqlQuery q(m_db);

    if (!q.exec("SELECT name FROM Authors ORDER BY name COLLATE NOCASE ASC"))
        return result;

    while (q.next())
        result << q.value(0).toString();

    return result;
}

bool database::takeBook(int userId, int bookId)
{
    QSqlDatabase conn = database::db();
    QSqlQuery q(conn);

    if (!conn.transaction()) {
        qDebug() << "takeBook: transaction start failed:" << conn.lastError().text();
        return false;
    }

    q.prepare("SELECT status FROM Books WHERE id = :bookId");
    q.bindValue(":bookId", bookId);

    if (!q.exec() || !q.next()) {
        qDebug() << "takeBook: SELECT status failed:" << q.lastError().text();
        conn.rollback();
        return false;
    }

    QString status = q.value(0).toString();
    if (status != "available") {
        qDebug() << "takeBook: book is not available, status =" << status;
        conn.rollback();
        return false;
    }

    q.prepare("INSERT INTO Loans (user_id, book_id, issue_date, due_date, status) "
              "VALUES (:userId, :bookId, DATE('now'), DATE('now', '+14 day'), 'active')");
    q.bindValue(":userId", userId);
    q.bindValue(":bookId", bookId);

    if (!q.exec()) {
        qDebug() << "takeBook: INSERT loan failed:" << q.lastError().text();
        conn.rollback();
        return false;
    }

    q.prepare("UPDATE Books SET status = 'loaned' WHERE id = :bookId");
    q.bindValue(":bookId", bookId);

    if (!q.exec()) {
        qDebug() << "takeBook: UPDATE book failed:" << q.lastError().text();
        conn.rollback();
        return false;
    }

    if (!conn.commit()) {
        qDebug() << "takeBook: commit failed:" << conn.lastError().text();
        conn.rollback();
        return false;
    }

    return true;
}


