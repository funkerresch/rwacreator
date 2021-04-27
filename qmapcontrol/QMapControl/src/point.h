/*
*
* This file is part of QMapControl,
* an open-source cross-platform map widget
*
* Copyright (C) 2007 - 2008 Kai Winter
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with QMapControl. If not, see <http://www.gnu.org/licenses/>.
*
* Contact e-mail: kaiwinter@gmx.de
* Program URL   : http://qmapcontrol.sourceforge.net/
*
*/

#ifndef POINT_H
#define POINT_H
#include <QWidget>
#include <QLineEdit>
#include <QPointF>
#include <QLineEdit>
#include <QLabel>
#include "geometry.h"
#include "rwautilities.h"
#include "rwalocation1.h"

namespace qmapcontrol
{
    class QmapPoint : public Geometry
    {
        Q_OBJECT

    public:
        friend class Layer;
        friend class LineString;

        RwaLocation1 *data;
        qreal X;
        qreal Y;
        QSize displaysize;

        //! sets where the point should be aligned
        enum Alignment
        {
            TopLeft, /*!< Align on TopLeft*/
            TopRight, /*!< Align on TopRight*/
            BottomLeft, /*!< Align on BottomLeft*/
            BottomRight,/*!< Align on BottomRight*/
            Middle, /*!< Align on Middle*/
            RectMiddle
        };

        /*
         * This constructor creates a Point with no image or widget.
         * @param x longitude
         * @param y latitude
         * @param name name of the point
         * @param alignment allignment of the point (Middle or TopLeft)
         */
        QmapPoint(qreal x, qreal y, QString name = QString(), enum Alignment alignment=Middle);

        QmapPoint(qreal x, qreal y, QPixmap* pixmap, RwaLocation1 *data = 0, enum Alignment alignment = Middle);

        virtual ~QmapPoint();

        void setGpsCoordinates();

        void addLineEditWidget(QLineEdit *lineEdit);

        void addLabelWidget(QLabel *label);

        //! returns the bounding box of the point
        /*!
         * The Bounding contains the coordinate of the point and its size.
         * The size is set, if the point contains a pixmap or a widget
         * @return the bounding box of the point
         */
        virtual QRectF boundingBox();

        qreal longitude() const;

        qreal latitude() const;

        QLineEdit *lineEdit(); // return lineEdit

        void setLineEditVisible(bool isVisible);

        void setLabelVisible(bool isVisible);

        QPointF coordinate() const;

        virtual QList<QmapPoint*> points();

        QWidget* widget();

        QPixmap* pixmap();

        void setPixmap(QPixmap *pixmap);

        //! Sets the zoom level on which the point�s pixmap gets displayed on full size
        /*!
         * Use this method to set a zoom level on which the pixmap gets displayed with its real size.
         * On zoomlevels below it will be displayed smaller, and on zoom levels thereover it will be displayed larger
         * @see setMinsize, setMaxsize
         * @param zoomlevel the zoomlevel on which the point will be displayed on full size
         */
        void setBaselevel(int zoomlevel);

        //! sets a minimal size for the pixmap
        /*!
         * When the point's pixmap should change its size on zooming, this method sets the minimal size.
         * @see setBaselevel
         * @param minsize the minimal size which the pixmap should have
         */
        void setMinsize(QSize minsize);

        //! sets a maximal size for the pixmap
        /*!
         * When the point´s pixmap should change its size on zooming, this method sets the maximal size.
         * @see setBaselevel
         * @param maxsize the maximal size which the pixmap should have
         */
        void setMaxsize(QSize maxsize);

        QmapPoint::Alignment alignment() const;

        bool getAllowTouches() const;
        void setAllowTouches(bool value);

       // int getType() const;
       // void setType(int value);

        RwaLocation1 *getData() const;
        void setData(RwaLocation1 *value);

        virtual void move(double dx, double dy);
    protected:

        QSize size;

        QWidget* mywidget;
        QLineEdit *mylineedit;
        QLabel *mylabel;
        QPixmap* mypixmap;
        Alignment myalignment;
        int homelevel;

        QSize minsize;
        QSize maxsize;
        QPen pen = QPen();
        bool allowTouches;

        void drawWidget(const MapAdapter* mapadapter, const QPoint offset);
        void drawLineEdit(const MapAdapter* mapadapter, const QPoint offset);
        void drawLabel(const MapAdapter* mapadapter, const QPoint offset);
        // void drawPixmap(QPainter* painter, const MapAdapter* mapadapter, const QRect &viewport, const QPoint versch);
        virtual void draw(QPainter* painter, const MapAdapter* mapadapter, const QRect &viewport, const QPoint offset);
        QPoint alignedPoint(const QPoint point) const;

        //! returns true if the given Point touches this Point
        /*!
         * The collision detection checks for the bounding rectangulars.
         * @param geom the other point which should be tested on collision
         * @param mapadapter the mapadapter which is used for calculations
         * @return
         */
        virtual bool Touches(QmapPoint* geom, const MapAdapter* mapadapter);

    public slots:
        void setCoordinate(QPointF point);
        void looseLineEditFocus();
        void setPenColor(QColor color);
        virtual void setVisible(bool visible);
    };
}
#endif
