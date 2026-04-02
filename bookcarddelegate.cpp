#include "bookcarddelegate.h"

#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

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

    QRect r = option.rect.adjusted(4, 4, -4, -4);

    QColor bg = (option.state & QStyle::State_Selected)
                    ? QColor("#e0f0ff")
                    : Qt::white;

    p->setBrush(bg);
    p->setPen(QColor("#cccccc"));
    p->drawRoundedRect(r, 8, 8);

    QRect coverRect(r.left() + 10, r.top() + 10, r.width() - 20, 190);
    QRect titleRect(r.left() + 10, coverRect.bottom() + 8, r.width() - 20, 36);
    QRect metaRect(r.left() + 10, titleRect.bottom() + 4, r.width() - 20, 18);
    QRect btnRect(r.left() + 20, r.bottom() - 36, r.width() - 40, 24);

    QPixmap pix = index.data(Qt::DecorationRole).value<QPixmap>();
    if (!pix.isNull()) {
        QPixmap scaled = pix.scaled(coverRect.size(),
                                    Qt::KeepAspectRatioByExpanding,
                                    Qt::SmoothTransformation);
        p->drawPixmap(coverRect, scaled,
                      QRect(0, 0, coverRect.width(), coverRect.height()));
    } else {
        p->fillRect(coverRect, QColor("#dbeafe"));
        p->setPen(Qt::darkGray);
        p->drawText(coverRect, Qt::AlignCenter, QObject::tr("No cover"));
    }

    QString title  = index.data(Qt::DisplayRole).toString();
    QString author = index.sibling(index.row(), 2).data().toString();
    QString year   = index.sibling(index.row(), 4).data().toString();
    QString meta   = QObject::tr("%1 • %2").arg(author, year);

    p->setPen(Qt::black);
    QFont titleFont = option.font;
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 1);
    p->setFont(titleFont);

    p->drawText(titleRect,
                Qt::AlignTop | Qt::AlignHCenter,
                p->fontMetrics().elidedText(title, Qt::ElideRight, titleRect.width()));

    QFont metaFont = option.font;
    metaFont.setPointSize(qMax(8, option.font.pointSize() - 1));
    metaFont.setBold(false);
    p->setFont(metaFont);
    p->setPen(QColor("#666666"));

    p->drawText(metaRect,
                Qt::AlignTop | Qt::AlignHCenter,
                p->fontMetrics().elidedText(meta, Qt::ElideRight, metaRect.width()));

    if (m_userRole == "user") {
        p->setBrush(QColor("#4caf50"));
        p->setPen(Qt::NoPen);
        p->drawRoundedRect(btnRect, 5, 5);

        p->setPen(Qt::white);
        p->setFont(option.font);
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
