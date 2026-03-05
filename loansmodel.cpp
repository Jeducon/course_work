#include "loansmodel.h"
#include "QPixmap"


QVariant LoansModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.column() == 1 && role == Qt::DecorationRole) {
        QString path = QSqlQueryModel::data(this->index(index.row(), 2),
                                            Qt::DisplayRole).toString();
        if (path.isEmpty())
            return QVariant();

        QPixmap pix(path);
        if (!pix.isNull())
            return pix.scaled(32, 48,
                              Qt::KeepAspectRatio,
                              Qt::SmoothTransformation);

        return QVariant();
    }

    return QSqlQueryModel::data(index, role);
}





