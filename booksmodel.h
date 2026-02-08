#ifndef BOOKSMODEL_H
#define BOOKSMODEL_H

#include <QSqlTableModel>

class booksmodel : public QSqlTableModel
{
    Q_OBJECT
public:
    explicit booksmodel(QObject *parent = nullptr,
                        const QSqlDatabase &db = QSqlDatabase());

    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;
};

#endif // BOOKSMODEL_H
