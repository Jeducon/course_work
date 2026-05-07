#pragma once

#ifndef BOOKCARDDELEGATE_H
#define BOOKCARDDELEGATE_H

#include <QStyledItemDelegate>
#include <QPixmap>

class BookCardDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit BookCardDelegate(QObject *parent = nullptr);

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

    void setUserRole(const QString &role) { m_userRole = role; }

    static void clearCoverCache();

signals:
    void bookInfoRequested(const QModelIndex &index);
    void takeRequested(const QModelIndex &index);

private:
    QString m_userRole;

    QPixmap cachedCover(const QModelIndex &index,
                        const QSize &targetSize) const;
};

#endif // BOOKCARDDELEGATE_H
