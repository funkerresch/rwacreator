#include "rwalistbadgedelegate.h"
#include "rwabackend.h"

#include <QPainter>
#include <QFile>

RwaListBadgeDelegate::RwaListBadgeDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

const QIcon &RwaListBadgeDelegate::badgeIcon(const QString &name) const
{
    auto cached = iconCache.constFind(name);
    if(cached != iconCache.constEnd())
        return cached.value();

    QString path = RwaBackend::getInstance()->completeBundlePath + "images/" + name + ".svg";
    if(QFile::exists(path))
        return iconCache.insert(name, QIcon(path)).value();

    return iconCache.insert(name, QIcon()).value();
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

    int spacing = 2;
    int x = option.rect.right() - spacing;
    int y = option.rect.center().y() - size / 2;

    // badges.first() ends up at the right edge, the rest follow to the left
    foreach(const QString &name, badges)
    {
        const QIcon &icon = badgeIcon(name);
        if(icon.isNull())
            continue;

        x -= size;
        icon.paint(painter, QRect(x, y, size, size));
        x -= spacing;

        if(x <= option.rect.left())
            break;
    }
}
