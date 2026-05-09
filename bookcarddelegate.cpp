#include "bookcarddelegate.h"

#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QPainterPath>
#include <QPixmapCache>
#include <QTextLayout>
#include <QAbstractTextDocumentLayout>
#include <QDir>
#include <QCoreApplication>


#include "booksmodel.h"

namespace {

QString resolveCoversPath(const QString &storedPath)
{
    if (storedPath.isEmpty())
        return QString();
    if (QDir::isAbsolutePath(storedPath))
        return storedPath;
    return QDir(QCoreApplication::applicationDirPath()).filePath(storedPath);
}

QString elideMultiLineText(const QString &text,
                           const QFont &font,
                           int width,
                           int maxLines)
{
    if (text.trimmed().isEmpty() || width <= 0 || maxLines <= 0)
        return text;

    QTextLayout layout(text, font);
    layout.beginLayout();

    QString result;
    int linesUsed = 0;
    int start = 0;

    while (linesUsed < maxLines) {
        QTextLine line = layout.createLine();
        if (!line.isValid())
            break;

        line.setLineWidth(width);
        const int lineStart = line.textStart();
        const int lineLength = line.textLength();
        QString piece = text.mid(lineStart, lineLength).trimmed();

        if (linesUsed == maxLines - 1 &&
            lineStart + lineLength < text.size()) {
            QFontMetrics fm(font);
            piece = fm.elidedText(text.mid(lineStart).trimmed(),
                                  Qt::ElideRight,
                                  width);
        }

        if (!result.isEmpty())
            result += '\n';
        result += piece;

        start = lineStart + lineLength;
        ++linesUsed;

        if (start >= text.size())
            break;
    }

    layout.endLayout();
    return result;
}
}

BookCardDelegate::BookCardDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QSize BookCardDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(210, 320);
}

void BookCardDelegate::clearCoverCache()
{
    QPixmapCache::clear();
}

QPixmap BookCardDelegate::cachedCover(const QModelIndex &index,
                                      const QSize &targetSize) const
{
    QString coverPath;

    QVariant v = index.data(booksmodel::CoverPathRole);
    if (v.isValid()) {
        coverPath = v.toString();
    } else {
        coverPath = index.sibling(index.row(), 7)
        .data(Qt::DisplayRole).toString();
    }

    if (coverPath.isEmpty())
        return QPixmap();

    const QString absPath = resolveCoversPath(coverPath);

    const QString cacheKey = QStringLiteral("bookcover:%1:%2x%3")
                                 .arg(coverPath)
                                 .arg(targetSize.width())
                                 .arg(targetSize.height());

    QPixmap cached;
    if (QPixmapCache::find(cacheKey, &cached))
        return cached;

    QPixmap original(absPath);
    if (original.isNull())
        return QPixmap();

    QPixmap scaled = original.scaled(targetSize,
                                     Qt::KeepAspectRatioByExpanding,
                                     Qt::SmoothTransformation);
    QPixmapCache::insert(cacheKey, scaled);
    return scaled;
}

void BookCardDelegate::paint(QPainter *p,
                             const QStyleOptionViewItem &opt,
                             const QModelIndex &index) const
{
    QStyleOptionViewItem option = opt;
    initStyleOption(&option, index);

    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);
    p->setRenderHint(QPainter::SmoothPixmapTransform, true);
    p->setRenderHint(QPainter::TextAntialiasing, true);

    QRect r = option.rect.adjusted(6, 6, -6, -6);

    const bool selected = option.state & QStyle::State_Selected;
    const bool hovered  = option.state & QStyle::State_MouseOver;

    QColor border = QColor("#d7e1db");
    if (selected)
        border = QColor("#7ea08c");
    if (hovered)
        border = QColor("#1f5c42");

    QPainterPath cardPath;
    cardPath.addRoundedRect(r, 14, 14);
    p->setClipPath(cardPath);
    p->fillPath(cardPath, Qt::white);

    QPixmap pix = cachedCover(index, r.size());
    if (!pix.isNull()) {
        QPoint topLeft(r.center().x() - pix.width() / 2,
                       r.center().y() - pix.height() / 2);

        p->setOpacity(0.6);
        p->drawPixmap(topLeft, pix);
        p->setOpacity(1.0);
    } else {
        p->fillRect(r, QColor("#e7f0ea"));
    }

    const int overlayHeight = 108;
    QRect overlayRect(r.left(), r.bottom() - overlayHeight, r.width(), overlayHeight);

    QLinearGradient fade(overlayRect.topLeft(), overlayRect.bottomLeft());
    fade.setColorAt(0.0, QColor(255, 255, 255, 90));
    fade.setColorAt(0.35, QColor(255, 255, 255, 170));
    fade.setColorAt(1.0, QColor(255, 255, 255, 242));
    p->fillRect(overlayRect, fade);

    p->setClipping(false);
    p->setPen(QPen(border, hovered ? 2 : 1));
    p->setBrush(Qt::NoBrush);
    p->drawRoundedRect(r, 14, 14);

    QRect titleRect(r.left() + 12,
                    overlayRect.top() + 14,
                    r.width() - 24,
                    40);

    QRect metaRect(r.left() + 12,
                   titleRect.bottom() + 5,
                   r.width() - 24,
                   18);

    QRect btnRect(r.left() + 22,
                  r.bottom() - 34,
                  r.width() - 44,
                  24);

    QString title  = index.data(Qt::DisplayRole).toString();
    QString author = index.data(booksmodel::AuthorRole).toString();
    QString year   = index.sibling(index.row(), 4).data().toString();
    QString meta   = QObject::tr("%1 • %2").arg(author, year);

    QFont titleFont = option.font;
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 1);
    p->setFont(titleFont);
    p->setPen(QColor("#1b241f"));

    QString fittedTitle = elideMultiLineText(title, titleFont, titleRect.width(), 2);
    p->drawText(titleRect,
                Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap,
                fittedTitle);

    QFont metaFont = option.font;
    metaFont.setPointSize(qMax(8, option.font.pointSize() - 1));
    metaFont.setBold(false);
    p->setFont(metaFont);
    p->setPen(QColor("#414845"));
    p->drawText(metaRect,
                Qt::AlignHCenter | Qt::AlignTop,
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

    QRect r = option.rect.adjusted(6, 6, -6, -6);

    const int overlayHeight = 108;
    QRect overlayRect(r.left(), r.bottom() - overlayHeight, r.width(), overlayHeight);

    QRect titleRect(r.left() + 12,
                    overlayRect.top() + 14,
                    r.width() - 24,
                    40);

    QRect metaRect(r.left() + 12,
                   titleRect.bottom() + 5,
                   r.width() - 24,
                   18);

    QRect btnRect(r.left() + 22,
                  r.bottom() - 34,
                  r.width() - 44,
                  24);

    QPoint pos = me->pos();

    if (m_userRole == "user" && btnRect.contains(pos)) {
        qDebug() << "editorEvent: click in btnRect, row" << index.row();
        emit takeRequested(index);
        return true;
    }

    if (r.contains(pos) && !btnRect.contains(pos)) {
        emit bookInfoRequested(index);
        return true;
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
