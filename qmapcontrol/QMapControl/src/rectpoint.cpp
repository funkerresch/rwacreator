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

#include "rectpoint.h"
namespace qmapcontrol
{
    RectPoint::RectPoint(qreal x, qreal y, int width, int height, QString name, Alignment alignment, QPen* pen)
        : QmapPoint(x, y, name, alignment)
    {
        size = QSize(width, height);
        mypixmap = new QPixmap(width+1, height+1);
        mypixmap->fill(Qt::transparent);
        myalignment = Middle;
        QPainter painter(mypixmap);
        if (pen != 0)
        {
            painter.setPen(*pen);
        }
        painter.drawRect(0, 0, width, height);


    }

    RectPoint::RectPoint(qreal x, qreal y, QPointF northWest, QPointF southEast, QString name, Alignment alignment)
        : QmapPoint(x, y, name, alignment)
    {
       // size = QSize(width, height);
        //mypixmap = new QPixmap(width+1, height+1);
        //mypixmap->fill(Qt::transparent);
        myalignment = Middle;
        size = QSize();
        this->northWest = northWest;
        this->southEast = southEast;
    }

    RectPoint::~RectPoint()
    {
        delete mypixmap;
    }

    void RectPoint::move(double dx, double dy)
    {
        // emit(updateRequest(this));
        // emit(updateRequest(QRectF(X, Y, size.width(), size.height())));
        X = X+dx;
        Y = Y+dy;
        northWest.setX(northWest.x()+dx);
        northWest.setY(northWest.y()+dy);
        southEast.setX(southEast.x()+dx);
        southEast.setY(southEast.y()+dy);
        // emit(updateRequest(this));
        //qDebug() << point.x();


        emit(updateRequest(QRectF(X, Y, size.width(), size.height())));
        emit(positionChanged(this));
    }

    void RectPoint::setPen(QPen* pen)
    {
        mypen = pen;
        mypixmap = new QPixmap(width+1, height+1);
        mypixmap->fill(Qt::transparent);
        QPainter painter(mypixmap);
        painter.setPen(*pen);
        painter.drawRect(this->X, this->Y, width, height);
    }
    int RectPoint::getHeight() const
    {
        return height;
    }
    
    void RectPoint::setHeight(int value)
    {
        height = value;
        mypixmap = new QPixmap(width+1, height+1);
        mypixmap->fill(Qt::transparent);
        QPainter painter(mypixmap);
        painter.drawRect(this->X, this->Y, width, height);
    }
    
    int RectPoint::getWidth() const
    {
        return width;
    }
    
    
    int RectPoint::getType() const
    {
        return type;
    }
    
    void RectPoint::setType(int value)
    {
        type = value;
    }
    
    
    void RectPoint::setWidth(int value)
    {

        this->width = value;
        mypixmap = new QPixmap(width+1, height+1);
        mypixmap->fill(Qt::transparent);
        QPainter painter(mypixmap);
        painter.drawRect(this->X, this->Y, width, height);
    }

    void RectPoint::draw(QPainter* painter, const MapAdapter* mapadapter, const QRect &viewport, const QPoint offset)
    {
        if (!visible)
            return;

        if (homelevel > 0)
        {

            //painter->setCompositionMode(QPainter::CompositionMode_Destination);
            int currentzoom = mapadapter->maxZoom() < mapadapter->minZoom() ? mapadapter->minZoom() - mapadapter->currentZoom() : mapadapter->currentZoom();

            // int currentzoom = mapadapter->getZoom();
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

       /* if(mylineedit)
        {
            drawLineEdit(mapadapter, offset);
        }*/


       // if (mypixmap !=0)
        {
            const QPointF c = QPointF(X, Y);
            QPoint point = mapadapter->coordinateToDisplay(c);
            QPoint topLeft = mapadapter->coordinateToDisplay(northWest);
            QPoint bottomRight = mapadapter->coordinateToDisplay(southEast);
            QRect displayRect = QRect(topLeft, bottomRight);
            size = QSize(displayRect.width()+1, displayRect.height()+1);

            if (viewport.contains(point))
            {
                painter->setPen(pen);
                painter->drawRect(displayRect);
                QPoint alignedtopleft = alignedPoint(point);
                //painter->drawPixmap(alignedtopleft.x(), alignedtopleft.y(), displaysize.width(), displaysize.height(), *mypixmap);
                if(mylineedit != NULL)
                {
                    if(!mylineedit->isVisible())
                    {
                        //painter->drawText(alignedtopleft.x(), alignedtopleft.y(), data->objectName());
                       if(mylabel->isVisible())
                            drawLabel(mapadapter, offset);
                    }
                    else
                        drawLineEdit(mapadapter, offset);
                }
            }
        }

        /*else if (mywidget!=0)
        {

            drawWidget(mapadapter, offset);
        }*/
    }
}
