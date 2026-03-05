#ifndef BOOKSMODEL_H
#define BOOKSMODEL_H

#include <QSqlTableModel>
#include <QPixmap>

class booksmodel : public QSqlTableModel
{
    Q_OBJECT
public:
    enum BookRoles {
        AuthorRole = Qt::UserRole + 1,
        YearRole,
        StatusRole,
        CoverPathRole,
        IdRole
    };
    explicit booksmodel(QObject *parent = nullptr,
                        const QSqlDatabase &db = QSqlDatabase());

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int bookIdAtRow(int row) const;
};

#endif // BOOKSMODEL_H
