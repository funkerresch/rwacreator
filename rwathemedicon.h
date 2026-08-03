/*
 * This file is part of the Rwa Creator.
 * An open-source cross-platform Middleware for creating interactive Soundwalks
 *
 * License: MIT
 *
 * rwathemedicon.h
 * QIcon whose SVG's neutral gray (#434343) is replaced with the palette's
 * button-text color at paint time in dark mode, and with the disabled color
 * for disabled buttons; in light mode the authored gray is kept (the palette
 * color would be plain black). Semantic colors in the SVG are left untouched.
 * Not for icons drawn onto the map: the map background does not change with
 * the theme.
 *
 */

#ifndef RWATHEMEDICON_H
#define RWATHEMEDICON_H

#include <QIcon>
#include <QColor>
#include <QPixmap>

QIcon rwaThemedIcon(const QString &svgPath);

// renders svgPath aspect-fitted and centered into a size*devicePixelRatio
// pixmap, with the neutral gray replaced by color; used by rwaThemedIcon
// and RwaListBadgeDelegate
QPixmap rwaRenderRecoloredSvg(const QString &svgPath, const QColor &color, const QSize &size, qreal devicePixelRatio);

#endif // RWATHEMEDICON_H
