#include "booksmodel.h"
#include <QPixmap>

booksmodel::booksmodel(QObject *parent, const QSqlDatabase &db)
    : QSqlTableModel(parent, db)
{
}

QVariant booksmodel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const int row = index.row();
    const int col = index.column();

    if (col == IdColumn && role == Qt::DisplayRole) {
        return QSqlTableModel::data(
            this->index(row, TitleColumn),
            Qt::DisplayRole
            );
    }

    if (role == AuthorRole) {
        return QSqlTableModel::data(
            this->index(row, AuthorColumn),
            Qt::DisplayRole
            );
    }

    if (role == YearRole) {
        return QSqlTableModel::data(
            this->index(row, YearColumn),
            Qt::DisplayRole
            );
    }

    if (role == StatusRole) {
        return QSqlTableModel::data(
            this->index(row, StatusColumn),
            Qt::DisplayRole
            );
    }

    if (role == CoverPathRole) {
        return QSqlTableModel::data(
            this->index(row, CoverPathColumn),
            Qt::DisplayRole
            );
    }

    if (role == IdRole) {
        return QSqlTableModel::data(
            this->index(row, IdColumn),
            Qt::DisplayRole
            );
    }

    return QSqlTableModel::data(index, role);
}

int booksmodel::bookIdAtRow(int row) const
{
    return QSqlTableModel::data(
               index(row, IdColumn),
               Qt::DisplayRole
               ).toInt();
}
