#pragma once

#ifndef BOOKSFILTERPROXYMODEL_H
#define BOOKSFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class BooksFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit BooksFilterProxyModel(QObject *parent = nullptr);

    void setAuthorFilter(const QString &author);
    void setGenreFilter(const QString &genre);
    void setYearFilter(const QString &year);
    void setSearchText(const QString &text);
protected:
    bool filterAcceptsRow(int sourceRow,
                          const QModelIndex &sourceParent) const override;
private:
    QString m_authorFilter;
    QString m_genreFilter;
    QString m_yearFilter;
    QString m_searchText;
};

#endif // BOOKSFILTERPROXYMODEL_H
