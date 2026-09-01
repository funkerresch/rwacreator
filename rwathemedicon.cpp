#include "rwathemedicon.h"

#include <QIconEngine>
#include <QGuiApplication>
#include <QStyleHints>
#include <QPalette>
#include <QPainter>
#include <QFile>
#include <QSvgRenderer>
#include <QPixmapCache>

QPixmap rwaRenderRecoloredSvg(const QString &svgPath, const QColor &color, const QSize &size, qreal devicePixelRatio, const QByteArray &fromColor)
{
    QPixmap pixmap;
    QFile file(svgPath);

    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray svg = file.readAll();
        if(color.isValid()) // invalid color = keep the authored colors
            svg.replace(fromColor, color.name().toLatin1()); // only fromColor follows, other (semantic) colors stay

        QSvgRenderer renderer(svg);
        if(renderer.isValid() && !renderer.defaultSize().isEmpty())
        {
            QSizeF iconSize = renderer.defaultSize();
            iconSize.scale(size.width(), size.height(), Qt::KeepAspectRatio);

            pixmap = QPixmap(qRound(size.width() * devicePixelRatio), qRound(size.height() * devicePixelRatio));
            pixmap.fill(Qt::transparent);

            QPainter iconPainter(&pixmap);
            QRectF target(QPointF((size.width() - iconSize.width()) / 2, (size.height() - iconSize.height()) / 2) * devicePixelRatio,
                          iconSize * devicePixelRatio);
            renderer.render(&iconPainter, target);
            iconPainter.end();
            pixmap.setDevicePixelRatio(devicePixelRatio);
        }
    }

    return pixmap;
}

class RwaThemedIconEngine : public QIconEngine
{
    public:
        explicit RwaThemedIconEngine(const QString &svgPath) : svgPath(svgPath) {}

        QIconEngine *clone() const override
        {
            return new RwaThemedIconEngine(svgPath);
        }

        void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override
        {
            qreal devicePixelRatio = painter->device() ? painter->device()->devicePixelRatio() : 1.0;
            painter->drawPixmap(rect, scaledPixmap(rect.size(), mode, state, devicePixelRatio));
        }

        QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override
        {
            return scaledPixmap(size, mode, state, 1.0);
        }

        QPixmap scaledPixmap(const QSize &size, QIcon::Mode mode, QIcon::State state, qreal scale) override
        {
            Q_UNUSED(state);

            // recolor only when the authored gray wouldn't read: dark scheme
            // or a disabled button. In light mode the palette's button-text
            // color is plain black — harsher than the authored #434343.
            QColor color;
            if(mode == QIcon::Disabled)
                color = QGuiApplication::palette().color(QPalette::Disabled, QPalette::ButtonText);
            else if(QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark)
                color = QGuiApplication::palette().color(QPalette::Normal, QPalette::ButtonText);

            QString key = QString("rwathemedicon|%1|%2|%3x%4|%5")
                    .arg(svgPath, color.isValid() ? color.name() : QStringLiteral("authored"))
                    .arg(size.width()).arg(size.height()).arg(scale);

            QPixmap pixmap;
            if(!QPixmapCache::find(key, &pixmap))
            {
                pixmap = rwaRenderRecoloredSvg(svgPath, color, size, scale);
                QPixmapCache::insert(key, pixmap);
            }

            return pixmap;
        }

    private:
        QString svgPath;
};

QIcon rwaThemedIcon(const QString &svgPath)
{
    return QIcon(new RwaThemedIconEngine(svgPath)); // QIcon takes ownership of the engine
}
