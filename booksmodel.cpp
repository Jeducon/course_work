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

    int row = index.row();
    int col = index.column();

    if (col == 0) {
        if (role == Qt::DecorationRole) {
            if (columnCount() <= 6)
                return QVariant();

            QString path = QSqlTableModel::data(this->index(row, 6)).toString();
            QPixmap pix(path);
            if (pix.isNull())
                return QVariant();

            return pix.scaled(160, 240,
                              Qt::KeepAspectRatio,
                              Qt::SmoothTransformation);
        }
        if (role == Qt::DisplayRole) {
            return QSqlTableModel::data(this->index(row, 1));
        }
    }

    if (role == AuthorRole)
        return QSqlTableModel::data(this->index(row, 2));
    if (role == YearRole)
        return QSqlTableModel::data(this->index(row, 4));
    if (role == StatusRole)
        return QSqlTableModel::data(this->index(row, 5));
    if (role == CoverPathRole)
        return QSqlTableModel::data(this->index(row, 6));
    if (role == IdRole)
        return QSqlTableModel::data(this->index(row, 0));

    return QSqlTableModel::data(index, role);
}
