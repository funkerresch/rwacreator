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

#ifndef POLYGONPOINT_H
#define POLYGONPOINT_H

#include "point.h"

namespace qmapcontrol
{
    class PolygonPoint : public QmapPoint
    {

    public:
        PolygonPoint(qreal x, qreal y, std::vector<std::vector<double>> *corners, QString name = QString(), Alignment alignment = Middle);
        void draw(QPainter* painter, const MapAdapter* mapadapter, const QRect &viewport, const QPoint offset);

        virtual ~PolygonPoint();

        int getType() const;
        void setType(int value);

        int getWidth() const;
        void setWidth(int value);

        int getHeight() const;
        void setHeight(int value);

    public slots:
        void move(double dx, double dy);
    private:
        int type;
        int width;
        int height;
        std::vector<std::vector<double>> *corners;
        std::vector<std::vector<double>> corners1;

    };
}
#endif
