#include "bookcarddelegate.h"

#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <qpainterpath.h>

BookCardDelegate::BookCardDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QSize BookCardDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(210, 320);
}

void BookCardDelegate::paint(QPainter *p,
                             const QStyleOptionViewItem &opt,
                             const QModelIndex &index) const
{
    QStyleOptionViewItem option = opt;
    initStyleOption(&option, index);

    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);

    QRect r = option.rect.adjusted(6, 6, -6, -6);

    const bool selected = option.state & QStyle::State_Selected;
    const bool hovered  = option.state & QStyle::State_MouseOver;

    QColor cardBg = QColor("#ffffff");
    QColor border = QColor("#d7e1db");

    if (selected)
        border = QColor("#7ea08c");
    if (hovered)
        border = QColor("#1f5c42");

    p->setBrush(cardBg);
    p->setPen(QPen(border, hovered ? 2 : 1));
    p->drawRoundedRect(r, 14, 14);

    QRect coverRect(r.left() + 12, r.top() + 12, r.width() - 24, 190);
    QRect titleRect(r.left() + 12, coverRect.bottom() + 10, r.width() - 24, 40);
    QRect metaRect(r.left() + 12, titleRect.bottom() + 6, r.width() - 24, 20);
    QRect btnRect(r.left() + 22, r.bottom() - 40, r.width() - 44, 28);

    QPainterPath clipPath;
    clipPath.addRoundedRect(coverRect, 10, 10);
    p->setClipPath(clipPath);

    QPixmap pix = index.data(Qt::DecorationRole).value<QPixmap>();
    if (!pix.isNull()) {
        QPixmap scaled = pix.scaled(coverRect.size(),
                                    Qt::KeepAspectRatioByExpanding,
                                    Qt::SmoothTransformation);
        p->drawPixmap(coverRect, scaled,
                      QRect(0, 0, coverRect.width(), coverRect.height()));
    } else {
        p->fillRect(coverRect, QColor("#e7f0ea"));
        p->setPen(QColor("#6b7d72"));
        p->drawText(coverRect, Qt::AlignCenter, QObject::tr("No cover"));
    }

    p->setClipping(false);

    QString title  = index.data(Qt::DisplayRole).toString();
    QString author = index.sibling(index.row(), 2).data().toString();
    QString year   = index.sibling(index.row(), 4).data().toString();
    QString meta   = QObject::tr("%1 • %2").arg(author, year);

    QFont titleFont = option.font;
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 1);
    p->setFont(titleFont);
    p->setPen(QColor("#1b241f"));
    p->drawText(titleRect,
                Qt::AlignTop | Qt::AlignHCenter,
                p->fontMetrics().elidedText(title, Qt::ElideRight, titleRect.width()));

    QFont metaFont = option.font;
    metaFont.setPointSize(qMax(8, option.font.pointSize() - 1));
    metaFont.setBold(false);
    p->setFont(metaFont);
    p->setPen(QColor("#6b736e"));
    p->drawText(metaRect,
                Qt::AlignTop | Qt::AlignHCenter,
                p->fontMetrics().elidedText(meta, Qt::ElideRight, metaRect.width()));

    if (m_userRole == "user") {
        p->setBrush(QColor("#1f5c42"));
        p->setPen(Qt::NoPen);
        p->drawRoundedRect(btnRect, 8, 8);

        QFont btnFont = option.font;
        btnFont.setBold(true);
        p->setFont(btnFont);
        p->setPen(Qt::white);
        p->drawText(btnRect, Qt::AlignCenter, QObject::tr("Взяти"));
    }

    p->restore();
}

bool BookCardDelegate::editorEvent(QEvent *event,
                                   QAbstractItemModel *model,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index)
{
    Q_UNUSED(model);

    if (!index.isValid())
        return false;

    if (event->type() != QEvent::MouseButtonRelease)
        return QStyledItemDelegate::editorEvent(event, model, option, index);

    auto *me = static_cast<QMouseEvent *>(event);
    if (me->button() != Qt::LeftButton)
        return QStyledItemDelegate::editorEvent(event, model, option, index);

    QRect r = option.rect.adjusted(4, 4, -4, -4);


    QRect coverRect(r.left() + 10, r.top() + 10, r.width() - 20, 190);
    QRect titleRect(r.left() + 10, coverRect.bottom() + 8, r.width() - 20, 32);
    QRect metaRect(r.left() + 10, titleRect.bottom() + 4, r.width() - 20, 18);
    QRect btnRect(r.left() + 20, r.bottom() - 34, r.width() - 40, 24);

    QPoint pos = me->pos();


    if (coverRect.contains(pos) || titleRect.contains(pos) || metaRect.contains(pos)) {
        emit bookInfoRequested(index);
        return true;
    }

    if (m_userRole == "user" && btnRect.contains(pos)) {
        qDebug() << "editorEvent: click in btnRect, row" << index.row();
        emit takeRequested(index);
        return true;
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
