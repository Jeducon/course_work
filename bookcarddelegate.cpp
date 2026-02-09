#include "bookcarddelegate.h"

#include <QPainter>

BookCardDelegate::BookCardDelegate(QObject *parent) : QStyledItemDelegate(parent)
{}

QSize BookCardDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(180, 280);
}

void BookCardDelegate::paint(QPainter *p,
                             const QStyleOptionViewItem &opt,
                             const QModelIndex &index) const
{
    QStyleOptionViewItem option = opt;
    initStyleOption(&option, index);

    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);

    QRect r = option.rect;

    QColor bg = (option.state & QStyle::State_Selected) ? QColor("#e0f0ff") : Qt::white;

    p->setBrush(bg);
    p->setPen(QColor("#cccccc"));
    p->drawRoundedRect(r.adjusted(2, 2, -2, -2), 6, 6);

    QPixmap pix = index.data(Qt::DecorationRole).value<QPixmap>();
    QRect coverRect(r.left() + 10, r.top() + 10, r.width() - 20, r.height() * 0.65);

    if(!pix.isNull())
        p->drawPixmap(coverRect, pix);

    QString title = index.data(Qt::DisplayRole).toString();
    QRect titleRect(r.left() + 10, coverRect.bottom() + 4, r.width() - 20, 40);

    p->setPen(Qt::black);
    QFont f = option.font;
    f.setBold(true);
    p->setFont(f);
    p->drawText(titleRect, Qt::AlignTop | Qt::AlignHCenter,
                p->fontMetrics().elidedText(title, Qt::ElideRight, titleRect.width()));

    if(m_userRole == "user"){
        QRect btnRect(r.left() + 20, r.bottom() - 34, r.width() - 40, 24);
        p->setBrush(QColor("#4caf50"));
        p->setPen(Qt::NoPen);
        p->drawRoundedRect(btnRect, 4, 4);

        p->setPen(Qt::white);
        p->drawText(btnRect, Qt::AlignCenter, QObject::tr("Взяти"));
    }

    p->restore();
}
