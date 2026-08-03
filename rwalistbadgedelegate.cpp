#include "rwalistbadgedelegate.h"
#include "rwabackend.h"

#include <QPainter>
#include <QFile>
#include <QSvgRenderer>

RwaListBadgeDelegate::RwaListBadgeDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

QPixmap RwaListBadgeDelegate::badgePixmap(const QString &name, const QColor &color, int size, qreal devicePixelRatio) const
{
    QString key = QString("%1|%2|%3|%4").arg(name, color.name()).arg(size).arg(devicePixelRatio);
    auto cached = pixmapCache.constFind(key);
    if(cached != pixmapCache.constEnd())
        return cached.value();

    QPixmap pixmap;
    QFile file(RwaBackend::getInstance()->completeBundlePath + "images/" + name + ".svg");

    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray svg = file.readAll();
        svg.replace("#434343", color.name().toLatin1()); // the neutral gray follows the palette, semantic colors stay

        QSvgRenderer renderer(svg);
        if(renderer.isValid() && !renderer.defaultSize().isEmpty())
        {
            QSizeF iconSize = renderer.defaultSize();
            iconSize.scale(size, size, Qt::KeepAspectRatio);

            pixmap = QPixmap(qRound(size * devicePixelRatio), qRound(size * devicePixelRatio));
            pixmap.fill(Qt::transparent);

            QPainter iconPainter(&pixmap);
            QRectF target(QPointF((size - iconSize.width()) / 2, (size - iconSize.height()) / 2) * devicePixelRatio,
                          iconSize * devicePixelRatio);
            renderer.render(&iconPainter, target);
            iconPainter.end();
            pixmap.setDevicePixelRatio(devicePixelRatio);
        }
    }

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

    QColor color = option.state & QStyle::State_Selected
            ? option.palette.color(QPalette::HighlightedText)
            : option.palette.color(QPalette::Text);
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
