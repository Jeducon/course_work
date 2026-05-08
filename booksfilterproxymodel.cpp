#include "booksfilterproxymodel.h"
#include "booksmodel.h"

BooksFilterProxyModel::BooksFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{}

void BooksFilterProxyModel::setAuthorFilter(const QString &author)
{
    m_authorFilter = author;
    invalidateFilter();
}

void BooksFilterProxyModel::setGenreFilter(const QString &genre)
{
    m_genreFilter = genre;
    invalidateFilter();
}

void BooksFilterProxyModel::setYearFilter(const QString &year)
{
    m_yearFilter = year;
    invalidateFilter();
}

void BooksFilterProxyModel::setSearchText(const QString &text)
{
    m_searchText = text;
    invalidateFilter();
}

bool BooksFilterProxyModel::filterAcceptsRow(int sourceRow,
                                             const QModelIndex &sourceParent) const
{
    auto *m = sourceModel();
    if (!m)
        return true;

    QString title  = m->data(m->index(sourceRow, 1, sourceParent)).toString();
    QString author = m->data(m->index(sourceRow, 2, sourceParent)).toString();
    QString genre  = m->data(m->index(sourceRow, 3, sourceParent)).toString();
    QString year   = m->data(m->index(sourceRow, 4, sourceParent)).toString();

    if (!m_searchText.isEmpty()) {
        const QString t = m_searchText.toLower();
        if (!title.toLower().contains(t) &&
            !author.toLower().contains(t) &&
            !genre.toLower().contains(t) &&
            !year.toLower().contains(t)) {
            return false;
        }
    }

    if (!m_authorFilter.isEmpty() &&
        !author.startsWith(m_authorFilter, Qt::CaseInsensitive))
        return false;

    if (!m_genreFilter.isEmpty() &&
        !genre.startsWith(m_genreFilter, Qt::CaseInsensitive))
        return false;

    if (!m_yearFilter.isEmpty() &&
        !year.startsWith(m_yearFilter))
        return false;

    return true;
}


