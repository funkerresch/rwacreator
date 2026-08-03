#include "rwalistbadgedelegate.h"
#include "rwabackend.h"
#include "rwathemedicon.h"

#include <QPainter>
#include <QGuiApplication>
#include <QStyleHints>

RwaListBadgeDelegate::RwaListBadgeDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

QPixmap RwaListBadgeDelegate::badgePixmap(const QString &name, const QColor &color, int size, qreal devicePixelRatio) const
{
    QString key = QString("%1|%2|%3|%4").arg(name, color.isValid() ? color.name() : QStringLiteral("authored")).arg(size).arg(devicePixelRatio);
    auto cached = pixmapCache.constFind(key);
    if(cached != pixmapCache.constEnd())
        return cached.value();

    QString path = RwaBackend::getInstance()->completeBundlePath + "images/" + name + ".svg";
    QPixmap pixmap = rwaRenderRecoloredSvg(path, color, QSize(size, size), devicePixelRatio);

    return pixmapCache.insert(key, pixmap).value();
}

void RwaListBadgeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    QStringList badges = index.data(BadgeRole).toStringList();
    if(badges.isEmpty())
        return;

    int size = qMin(option.rect.height() - 2, 16);
    if(size <= 0)
        return;

    // recolor only when the authored gray wouldn't read: on the selection
    // highlight or in dark mode. In light mode the palette's text color is
    // plain black. State_Active is cleared while the view has no focus —
    // the selection bar is then gray, not blue, and the icons follow the
    // text back to its unselected color.
    QColor color;
    if((option.state & QStyle::State_Selected) && (option.state & QStyle::State_Active))
        color = option.palette.color(QPalette::HighlightedText);
    else if(QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark)
        color = option.palette.color(QPalette::Text);
    qreal devicePixelRatio = painter->device() ? painter->device()->devicePixelRatio() : 1.0;

    int spacing = 2;
    int x = option.rect.right() - spacing;
    int y = option.rect.center().y() - size / 2;

    // badges.first() ends up at the right edge, the rest follow to the left
    foreach(const QString &name, badges)
    {
        QPixmap pixmap = badgePixmap(name, color, size, devicePixelRatio);
        if(pixmap.isNull())
            continue;

        x -= size;
        painter->drawPixmap(QRect(x, y, size, size), pixmap);
        x -= spacing;

        if(x <= option.rect.left())
            break;
    }
}
