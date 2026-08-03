/*
 * This file is part of the Rwa Creator.
 * An open-source cross-platform Middleware for creating interactive Soundwalks
 *
 * Copyright (C) 2015 - 2022 Thomas Resch
 *
 * License: MIT
 *
 * rwalistbadgedelegate.h
 * Item delegate for the list views: paints a row of small status icons
 * ("badges") right-aligned over the item text. Which badges an item shows
 * is controlled by the list that owns the items: it stores a QStringList
 * of icon basenames (without path or ".svg") under BadgeRole. Names that
 * don't resolve to an existing icon file are skipped, so lists can set
 * badge names before the corresponding icons ship.
 *
 * The icons' neutral gray (#434343) is replaced with the palette's text
 * color at render time, so badges stay visible in dark mode and on the
 * selection highlight; semantic colors in the SVGs are left untouched.
 *
 */

#ifndef RWALISTBADGEDELEGATE_H
#define RWALISTBADGEDELEGATE_H

#include <QStyledItemDelegate>
#include <QHash>
#include <QPixmap>

class RwaListBadgeDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    public:
        static constexpr int BadgeRole = Qt::UserRole + 1;

        explicit RwaListBadgeDelegate(QObject *parent = nullptr);
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    private:
        QPixmap badgePixmap(const QString &name, const QColor &color, int size, qreal devicePixelRatio) const;
        mutable QHash<QString, QPixmap> pixmapCache;
};

#endif // RWALISTBADGEDELEGATE_H
