#pragma once

#ifndef BOOKCARDDELEGATE_H
#define BOOKCARDDELEGATE_H

#include <QStyledItemDelegate>

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

signals:
    void bookInfoRequested(const QModelIndex &index);
    void takeRequested(const QModelIndex &index);

public:
    void setUserRole(const QString &role) { m_userRole = role;}

private:
    QString m_userRole;
};

#endif // BOOKCARDDELEGATE_H
