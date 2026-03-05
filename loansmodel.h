#pragma once

#ifndef LOANSMODEL_H
#define LOANSMODEL_H

#include <QSqlQueryModel>
#include <QPixmap>

class LoansModel : public QSqlQueryModel
{
    Q_OBJECT
public:
    explicit LoansModel(QObject *parent = nullptr) : QSqlQueryModel(parent)
    {}

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};

#endif // LOANSMODEL_H
