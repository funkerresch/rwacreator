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

#include "polygonpoint.h"
namespace qmapcontrol
{
    PolygonPoint::PolygonPoint(qreal x, qreal y, std::vector<std::vector<double>>*corners, QString name, Alignment alignment)
        : QmapPoint(x, y, name, alignment)
    {
         // this->corners = corners; // I passed the vector from rwalocation here which is deleted on loading new files or undos
          myalignment = Middle;
          size = QSize();

          foreach(std::vector<double> corner, *corners) // so we have to create a real copy of the polygon for drawing
              corners1.push_back(std::vector<double>(corner));

          this->corners = &corners1;
    }

    PolygonPoint::~PolygonPoint()
    {
        delete mypixmap;
        //delete corners;
    }

    int PolygonPoint::getHeight() const
    {
        return height;
    }

    void PolygonPoint::setHeight(int value)
    {
        height = value;
        mypixmap = new QPixmap(width+1, height+1);
        mypixmap->fill(Qt::transparent);
        QPainter painter(mypixmap);
        painter.drawRect(this->X, this->Y, width, height);
    }

    int PolygonPoint::getWidth() const
    {
        return width;
    }


    int PolygonPoint::getType() const
    {
        return type;
    }

    void PolygonPoint::setType(int value)
    {
        type = value;
    }

    void PolygonPoint::setWidth(int value)
    {
        this->width = value;
        mypixmap = new QPixmap(width+1, height+1);
        mypixmap->fill(Qt::transparent);
        QPainter painter(mypixmap);
        painter.drawRect(this->X, this->Y, width, height);
    }

    void PolygonPoint::move(double dx, double dy)
    {
        uint64_t i = 0;
        while(i < corners1.size())
        {
            corners->at(i)[0] += dx;
            corners->at(i)[1] += dy;
            i++;
        }

        emit(updateRequest(QRectF(X, Y, size.width(), size.height())));
        emit(positionChanged(this));
    }

    void PolygonPoint::draw(QPainter* painter, const MapAdapter* mapadapter, const QRect &viewport, const QPoint offset)
    {
        if (!visible)
            return;

        if (homelevel > 0)
        {
            int currentzoom = mapadapter->maxZoom() < mapadapter->minZoom() ? mapadapter->minZoom() - mapadapter->currentZoom() : mapadapter->currentZoom();
            int diffzoom = homelevel-currentzoom;
            int viewheight = size.height();
            int viewwidth = size.width();
            viewheight = int(viewheight / pow(2, diffzoom));
            viewwidth = int(viewwidth / pow(2, diffzoom));

            if (minsize.height()!= -1 && viewheight < minsize.height())
                viewheight = minsize.height();
            else if (maxsize.height() != -1 && viewheight > maxsize.height())
                viewheight = maxsize.height();

            if (minsize.width()!= -1 && viewwidth < minsize.width())
                viewwidth = minsize.width();
            else if (maxsize.width() != -1 && viewwidth > maxsize.width())
                viewwidth = maxsize.width();

            displaysize = QSize(viewwidth, viewheight);
        }
        else
        {
            displaysize = size;
        }

        //if (mypixmap !=0)
        {
            QVector <QPoint> v = QVector <QPoint>();
            for(int i = 0; i < corners->size(); i++)
            {
                QPointF tmp(0,0);
                tmp.setX((*corners)[i][0]);
                tmp.setY((*corners)[i][1]);
                v.append(mapadapter->coordinateToDisplay(tmp));
            }
            const QPointF c = QPointF(X, Y);
            QPoint point = mapadapter->coordinateToDisplay(c);

            QPolygon pol(v);
            QRect boundingRect = pol.boundingRect();
            size = QSize(boundingRect.width()+20, boundingRect.height()+20);

            painter->setPen(pen);

            //if (viewport.contains(point))
            {             
                painter->drawPolygon(pol);
                for(int i = 0; i < corners->size(); i++)
                    painter->drawEllipse(v.at(i),2,2);

               /* if(mylineedit != NULL)
                {
                    if(!mylineedit->isVisible())
                    {
                       if(mylabel->isVisible())
                            drawLabel(mapadapter, offset);
                    }
                    else
                        drawLineEdit(mapadapter, offset);
                }*/
            }
        }
    }
}
