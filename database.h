#pragma once

#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QStringList>

class database
{
public:
    static bool init(const QString &path);
    static bool takeBook(int userId, int bookId);
    static QSqlDatabase db();

    static int ensureAuthorExists(const QString &authorName);
    static QString authorNameById(int authorId);
    static QStringList allAuthorNames();

private:
    static bool migrateBooksAuthorToAuthors();
    static QSqlDatabase m_db;
};

#endif
