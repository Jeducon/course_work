#ifndef BOOKSMODEL_H
#define BOOKSMODEL_H

#include <QSqlTableModel>
#include <QHash>

class booksmodel : public QSqlTableModel
{
    Q_OBJECT

public:
    enum BookColumns {
        IdColumn = 0,
        TitleColumn = 1,
        AuthorIdColumn = 2,
        GenreColumn = 3,
        YearColumn = 4,
        StatusColumn = 5,
        DescriptionColumn = 6,
        CoverPathColumn = 7
    };

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
    QString authorNameAtRow(int row) const;
    bool refreshAuthorCache();

private:
    mutable QHash<int, QString> m_authorCache;
};

#endif // BOOKSMODEL_H
