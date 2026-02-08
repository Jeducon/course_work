#include "booksmodel.h"
#include "QPixmap"
booksmodel::booksmodel(QObject *parent, const QSqlDatabase &db) : QSqlTableModel(parent, db){}

QVariant booksmodel::data(const QModelIndex &index, int role) const
{
    if(index.column() == 0){
        if(role == Qt::DecorationRole){
            QString path = QSqlTableModel::data(
                this->index(index.row(), 6),
                               Qt::DisplayRole).toString();
            QPixmap pix(path);
            if(!pix.isNull())
                return pix.scaled(48, 72, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        if(role == Qt::DisplayRole)
            return QString();
    }

    return QSqlTableModel::data(index, role);
}
