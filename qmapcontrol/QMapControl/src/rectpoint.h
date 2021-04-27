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

#ifndef RECTPOINT_H
#define RECTPOINT_H

#include "point.h"

namespace qmapcontrol
{
    //! Draws a circle into the map
    /*! This is a conveniece class for Point.
     * It configures the pixmap of a Point to draw a circle.
     * A QPen could be used to change the color or line-width of the circle
     *
     * @author Kai Winter <kaiwinter@gmx.de>
     */
    class RectPoint : public QmapPoint
    {
    public:

        QPointF northWest;
        QPointF southEast;
        //!
        /*!
         *
         * @param x longitude
         * @param y latitude
         * @param radius the radius of the circle
         * @param name name of the circle point
         * @param alignment alignment (Middle or TopLeft)
         * @param pen QPen for drawing
         */
        RectPoint(qreal x, qreal y, int width = 10, int height = 10, QString name = QString(), Alignment alignment = Middle, QPen* pen=0);
        RectPoint(qreal x, qreal y, QPointF northWest, QPointF southEast, QString name = QString(), Alignment alignment = Middle);
        void draw(QPainter* painter, const MapAdapter* mapadapter, const QRect &viewport, const QPoint offset);

        virtual ~RectPoint();

        //! sets the QPen which is used for drawing the circle
        /*!
         * A QPen can be used to modify the look of the drawn circle
         * @param pen the QPen which should be used for drawing
         * @see http://doc.trolltech.com/4.3/qpen.html
         */
        virtual void setPen(QPen* pen);

        int getType() const;
        void setType(int value);

        int getWidth() const;
        void setWidth(int value);

        int getHeight() const;
        void setHeight(int value);


        void move(double dx, double dy);
    private:
        int type;
        int width;
        int height;

    };
}
#endif
