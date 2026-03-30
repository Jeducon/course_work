#pragma once

#ifndef DATABASE_H
#define DATABASE_H

#include <QtSql/QSqlDatabase>

class database
{
public:
    static bool init(const QString &path);
    static bool takeBook(int userId, int bookId);
    static QSqlDatabase db();

private:
    static QSqlDatabase m_db;
};

#endif
