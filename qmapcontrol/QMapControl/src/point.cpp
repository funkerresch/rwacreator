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

#include "point.h"
#include "rwastyles.h"

namespace qmapcontrol
{
    QmapPoint::QmapPoint(qreal x, qreal y, QString name, enum Alignment alignment) // THIS IS THE ONE IN USE..
        : Geometry(name), X(x), Y(y), myalignment(alignment)
    {
       // type = 0;
        GeometryType = "Point";
        mywidget = 0;
        mylineedit = 0;
        mypixmap = 0;
        visible = true;
        homelevel = -1;
        minsize = QSize(-1,-1);
        maxsize = QSize(-1,-1);
        data = NULL;
        mylineedit = NULL;
        mylabel = NULL;
        allowTouches = true;
        setPenColor(QColor(0,0,0,255));
    }

    QmapPoint::QmapPoint(qreal x, qreal y, QPixmap* pixmap, RwaLocation1 *state, enum Alignment alignment)
        : Geometry(QString::fromStdString(state->objectName())), X(x), Y(y), mypixmap(pixmap), myalignment(alignment)
    {
       // type = 0;
        GeometryType = "Point";
        mywidget = 0;
        mylineedit = 0;
        this->data = state;
        visible = true;
        size = pixmap->size();
        homelevel = -1;
        minsize = QSize(-1,-1);
        maxsize = QSize(-1,-1);
        mylineedit = NULL;
        mylabel = NULL;
        allowTouches = true;
        setPenColor(QColor(0,0,0,255));
    }

    RwaLocation1 *QmapPoint::getData() const
    {
        return data;
    }

    void QmapPoint::setData(RwaLocation1 *value)
    {
        data = value;
    }

    void QmapPoint::setPixmap(QPixmap *pixmap)
    {
        mypixmap = pixmap;
        size = pixmap->size();
    }

    void QmapPoint::addLineEditWidget(QLineEdit *lineEdit)
    {
        mylineedit = lineEdit;
        mylineedit->setText(QString::fromStdString(data->objectName()));
        mylineedit->setVisible(false);
        mylineedit->setStyleSheet(rwaSmallLineEditStyle);
        //connect(mylineedit, SIGNAL(returnPressed()), this, SLOT(looseLineEditFocus()));
    }

    void QmapPoint::addLabelWidget(QLabel *label)
    {
        mylabel = label;
        mylabel->setText(QString::fromStdString(data->objectName()));
        mylabel->setVisible(false);
        mylabel->setStyleSheet(rwaInactiveLableStyle);
    }

    void QmapPoint::setLineEditVisible(bool isVisible)
    {
        if(mylineedit != NULL)
            mylineedit->setVisible(isVisible);
    }

    void QmapPoint::setLabelVisible(bool isVisible)
    {
        if(mylabel != NULL)
            mylabel->setVisible(isVisible);
    }

    void QmapPoint::looseLineEditFocus()
    {
        if(mylineedit != NULL)
            mylineedit->clearFocus();
    }

    void QmapPoint::setPenColor(QColor color)
    {
        pen.setColor(color);
    }

    QLineEdit *QmapPoint::lineEdit()
    {
        return mylineedit;
    }

    void   QmapPoint::setGpsCoordinates()
    {
        ;//state->setGpsCoordinates(QPointF(this->X,this->Y));

    }

    QmapPoint::~QmapPoint()
    {
       /* if(mywidget)
            delete mywidget;
        if(mylineedit)
            delete mylineedit;
        if(mypixmap)
            delete mypixmap;*/
    }

    void QmapPoint::setVisible(bool visible)
    {
        this->visible = visible;
        if (mywidget !=0)
        {
            mywidget->setVisible(visible);

        }
        if(mylineedit != 0)
            mylineedit->setVisible(visible);
    }

    QRectF QmapPoint::boundingBox()
    {
        //TODO: have to be calculated in relation to alignment...
        return QRectF(QPointF(X, Y), displaysize);
    }

    qreal QmapPoint::longitude() const
    {
        return X;
    }
    qreal QmapPoint::latitude() const
    {
        return Y;
    }
    QPointF QmapPoint::coordinate() const
    {
        return QPointF(X, Y);
    }

    void QmapPoint::draw(QPainter* painter, const MapAdapter* mapadapter, const QRect &viewport, const QPoint offset)
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

        if (mypixmap !=0)
        {
            const QPointF c = QPointF(X, Y);
            QPoint point = mapadapter->coordinateToDisplay(c);

           // if (viewport.contains(point))
            {
                QPoint alignedtopleft = alignedPoint(point);
                painter->drawPixmap(alignedtopleft.x(), alignedtopleft.y(), displaysize.width(), displaysize.height(), *mypixmap);

                if(mylineedit)
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

        else if (mywidget!=0)
        {

            drawWidget(mapadapter, offset);
        }
    }

    void QmapPoint::drawLineEdit(const MapAdapter* mapadapter, const QPoint offset)
    {
        const QPointF c = QPointF(X, Y);
        QPoint point = mapadapter->coordinateToDisplay(c);
        point -= offset;

        QPoint alignedtopleft = alignedPoint(point);
        mylineedit->setGeometry(alignedtopleft.x(), alignedtopleft.y()+20, 100, 20);
    }

    void QmapPoint::drawLabel(const MapAdapter* mapadapter, const QPoint offset)
    {
        const QPointF c = QPointF(X, Y);
        QPoint point = mapadapter->coordinateToDisplay(c);
        point -= offset;

        QPoint alignedtopleft = alignedPoint(point);
        mylabel->setGeometry(alignedtopleft.x(), alignedtopleft.y()+20, 100, 20);
    }

    void QmapPoint::drawWidget(const MapAdapter* mapadapter, const QPoint offset)
    {
        const QPointF c = QPointF(X, Y);
        QPoint point = mapadapter->coordinateToDisplay(c);
        point -= offset;

        QPoint alignedtopleft = alignedPoint(point);
        mywidget->setGeometry(alignedtopleft.x(), alignedtopleft.y(), displaysize.width(), displaysize.height());
    }

    QPoint QmapPoint::alignedPoint(const QPoint point) const
    {
        QPoint alignedtopleft;
        //if (myalignment == Middle)
        {
            alignedtopleft.setX(point.x()-displaysize.width()/2);
            alignedtopleft.setY(point.y()-displaysize.height()/2);
        }

//       if(myalignment == RectMiddle)
//       {
//           alignedtopleft.setX(point.x()-displaysize.width()/2);
//           alignedtopleft.setY(point.y()-displaysize.height()/2);
//       }
//        else if (myalignment == TopLeft)
//        {
//            alignedtopleft.setX(point.x());
//            alignedtopleft.setY(point.y());
//        }
//        else if (myalignment == TopRight)
//        {
//            alignedtopleft.setX(point.x()-displaysize.width());
//            alignedtopleft.setY(point.y());
//        }
//        else if (myalignment == BottomLeft)
//        {
//            alignedtopleft.setX(point.x());
//            alignedtopleft.setY(point.y()-displaysize.height());
//        }
//        else if (myalignment == BottomRight)
//        {
//            alignedtopleft.setX(point.x()-displaysize.width());
//            alignedtopleft.setY(point.y()-displaysize.height());
//        }
        return alignedtopleft;
    }


    bool QmapPoint::Touches(QmapPoint* p, const MapAdapter* mapadapter)
    {
        if (this->isVisible() == false)
            return false;
        if (mypixmap == 0)
            return false;

        QPointF c = p->coordinate();
        // coordinate to pixel
        QPoint pxOfPoint = mapadapter->coordinateToDisplay(c);
        // size/2 Pixel toleranz aufaddieren
        QPoint p1;
        QPoint p2;

        switch (myalignment)
        {
                        case Middle:
            p1 = pxOfPoint - QPoint(displaysize.width()/2,displaysize.height()/2);
            p2 = pxOfPoint + QPoint(displaysize.width()/2,displaysize.height()/2);
            break;

                        case RectMiddle:
            p1 = pxOfPoint - QPoint(displaysize.width()/2,displaysize.height()/2);
            p2 = pxOfPoint + QPoint(displaysize.width()/2,displaysize.height()/2);
            break;
                        case TopLeft:
            p1 = pxOfPoint - QPoint(displaysize.width(),displaysize.height());
            p2 = pxOfPoint;
            break;
                        case TopRight:
            p1 = pxOfPoint - QPoint(0, displaysize.height());
            p2 = pxOfPoint + QPoint(displaysize.width(),0);
            break;
                        case BottomLeft:
            p1 = pxOfPoint - QPoint(displaysize.width(), 0);
            p2 = pxOfPoint + QPoint(0, displaysize.height());
            break;
                        case BottomRight:
            p1 = pxOfPoint;
            p2 = pxOfPoint + QPoint(displaysize.width(), displaysize.height());
            break;
        }

        // calculate "Bounding Box" in coordinates
        QPointF c1 = mapadapter->displayToCoordinate(p1);
        QPointF c2 = mapadapter->displayToCoordinate(p2);


        if(this->longitude()>=c1.x() && this->longitude()<=c2.x())
        {
            if (this->latitude()<=c1.y() && this->latitude()>=c2.y())
            {
                emit(geometryClicked(this, QPoint(0,0)));
                return true;
            }
        }
        return false;
    }

    void QmapPoint::setCoordinate(QPointF point)
    {
        // emit(updateRequest(this));
        // emit(updateRequest(QRectF(X, Y, size.width(), size.height())));
        X = point.x();
        Y = point.y();
        // emit(updateRequest(this));
        //qDebug() << point.x();


        emit(updateRequest(QRectF(X, Y, size.width(), size.height())));
        emit(positionChanged(this));
    }

    void QmapPoint::move(double dx, double dy)
    {
        // emit(updateRequest(this));
        // emit(updateRequest(QRectF(X, Y, size.width(), size.height())));
        X = X+dx;
        Y = Y+dy;
        // emit(updateRequest(this));
        //qDebug() << point.x();


        emit(updateRequest(QRectF(X, Y, size.width(), size.height())));
        emit(positionChanged(this));
    }

    QList<QmapPoint*> QmapPoint::points()
    {
        //TODO: assigning temp?!
        QList<QmapPoint*> points;
        points.append(this);
        return points;
    }

    QWidget* QmapPoint::widget()
    {
        return mywidget;
    }

    QPixmap* QmapPoint::pixmap()
    {
        return mypixmap;
    }

    void QmapPoint::setBaselevel(int zoomlevel)
    {
        homelevel = zoomlevel;
    }
    void QmapPoint::setMinsize(QSize minsize)
    {
        this->minsize = minsize;
    }
    void QmapPoint::setMaxsize(QSize maxsize)
    {
        this->maxsize = maxsize;
    }
    QmapPoint::Alignment QmapPoint::alignment() const
    {
        return myalignment;
    }
    bool QmapPoint::getAllowTouches() const
    {
        return allowTouches;
    }
    
    void QmapPoint::setAllowTouches(bool value)
    {
        allowTouches = value;
    }
    
    
    
    
}
