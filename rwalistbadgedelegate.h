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
 */

#ifndef RWALISTBADGEDELEGATE_H
#define RWALISTBADGEDELEGATE_H

#include <QStyledItemDelegate>
#include <QHash>
#include <QIcon>

class RwaListBadgeDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    public:
        static constexpr int BadgeRole = Qt::UserRole + 1;

        explicit RwaListBadgeDelegate(QObject *parent = nullptr);
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    private:
        const QIcon &badgeIcon(const QString &name) const;
        mutable QHash<QString, QIcon> iconCache;
};

#endif // RWALISTBADGEDELEGATE_H
