#include "booksmodel.h"

#include <QSqlQuery>
#include <QVariant>

booksmodel::booksmodel(QObject *parent, const QSqlDatabase &db)
    : QSqlTableModel(parent, db)
{
    refreshAuthorCache();
}

bool booksmodel::refreshAuthorCache()
{
    m_authorCache.clear();

    QSqlQuery q(database());
    if (!q.exec("SELECT id, name FROM Authors")) {
        return false;
    }

    while (q.next()) {
        m_authorCache.insert(q.value(0).toInt(), q.value(1).toString());
    }

    return true;
}

QVariant booksmodel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const int row = index.row();
    const int col = index.column();

    if (col == IdColumn && role == Qt::DisplayRole) {
        return QSqlTableModel::data(this->index(row, TitleColumn), Qt::DisplayRole);
    }

    if (col == AuthorIdColumn && role == Qt::DisplayRole) {
        const int authorId = QSqlTableModel::data(this->index(row, AuthorIdColumn), Qt::DisplayRole).toInt();
        return m_authorCache.value(authorId);
    }

    if (role == AuthorRole) {
        const int authorId = QSqlTableModel::data(this->index(row, AuthorIdColumn), Qt::DisplayRole).toInt();
        return m_authorCache.value(authorId);
    }

    if (role == YearRole) {
        return QSqlTableModel::data(this->index(row, YearColumn), Qt::DisplayRole);
    }

    if (role == StatusRole) {
        return QSqlTableModel::data(this->index(row, StatusColumn), Qt::DisplayRole);
    }

    if (role == CoverPathRole) {
        return QSqlTableModel::data(this->index(row, CoverPathColumn), Qt::DisplayRole);
    }

    if (role == IdRole) {
        return QSqlTableModel::data(this->index(row, IdColumn), Qt::DisplayRole);
    }

    return QSqlTableModel::data(index, role);
}

int booksmodel::bookIdAtRow(int row) const
{
    return QSqlTableModel::data(index(row, IdColumn), Qt::DisplayRole).toInt();
}

QString booksmodel::authorNameAtRow(int row) const
{
    const int authorId = QSqlTableModel::data(index(row, AuthorIdColumn), Qt::DisplayRole).toInt();
    return m_authorCache.value(authorId);
}
