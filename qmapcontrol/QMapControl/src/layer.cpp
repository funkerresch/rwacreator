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

#include "layer.h"


namespace qmapcontrol
{
    Layer::Layer(QString layername, MapAdapter* mapadapter, enum LayerType layertype, bool takeevents)
        :visible(true), mylayername(layername), mylayertype(layertype), mapAdapter(mapadapter), takeevents(takeevents), myoffscreenViewport(QRect(0,0,0,0))
    {
        selectRect.setX(0);
        selectRect.setY(0);
        selectRect.setWidth(0);
        selectRect.setHeight(0);
    }
    Layer::Layer()
    {
        mylayername = "";

    }

    Layer::~Layer()
    {
        delete mapAdapter;
    }

    void Layer::setActivePixmap(QPixmap activePixmap)
    {
        this->activePixmap = activePixmap;
    }

    QPixmap *Layer::getActivePixmap()
    {
        return &activePixmap;
    }

    QPixmap *Layer::getPassivePixmap()
    {
        return &passivePixmap;
    }

    QPixmap *Layer::getPixmap3()
    {
        return &pixmap3;
    }

    QPixmap *Layer::getPixmap4()
    {
        return &pixmap4;
    }

    QPixmap *Layer::getPixmap5()
    {
        return &pixmap5;
    }

    void Layer::setPassivePixmap(QPixmap passivePixmap)
    {
        this->passivePixmap = passivePixmap;
    }

    void Layer::setPixmap3(QPixmap pixmap3)
    {
        this->pixmap3 = pixmap3;
    }

    void Layer::setPixmap4(QPixmap pixmap4)
    {
        this->pixmap4 = pixmap4;
    }

    void Layer::setPixmap5(QPixmap pixmap5)
    {
        this->pixmap5 = pixmap5;
    }


    void Layer::setSize(QSize size)
    {
        this->size = size;
        screenmiddle = QPoint(size.width()/2, size.height()/2);
        //QMatrix mat;
        //mat.translate(480/2, 640/2);
        //mat.rotate(45);
        //mat.translate(-480/2,-640/2);
        //screenmiddle = mat.map(screenmiddle);
    }

    QString Layer::layername() const
    {
        return mylayername;
    }

    const MapAdapter* Layer::mapadapter() const
    {
        return mapAdapter;
    }

    void Layer::setVisible(bool visible)
    {
        this->visible = visible;
        emit updateRequest();
    }

    void Layer::addGeometry(Geometry* geom)
    {
        //qDebug() << geom->getName() << ", " << geom->getPoints().at(0)->getWidget();

       // geometries.append(QSharedPointer<Geometry>(geom));
        geometries.append(geom);
        emit updateRequest(geom->boundingBox());
        //a geometry can request a redraw, e.g. when its position has been changed
        connect(geom, SIGNAL(updateRequest(QRectF)),
                this, SIGNAL(updateRequest(QRectF)));
    }
    void Layer::removeGeometry(Geometry* geometry)
    {
        for (int i=0; i<geometries.count(); i++)
        {
            if (geometry == geometries.at(i))
            {
                QmapPoint *myPoint = (QmapPoint *)geometry;
                if(myPoint->mylineedit != NULL)
                    delete myPoint->mylineedit;
                if(myPoint->mylabel != NULL)
                    delete myPoint->mylabel;
                disconnect(geometry);
                geometries.removeOne(geometry);
                //delete geometry;
                break;
            }
        }
    }

    void Layer::clearGeometries()
    {
        //foreach(QSharedPointer<Geometry> geometry, geometries)
        foreach(Geometry *geometry, geometries)
        {
            //disconnect(geometry);
            if(geometry->GeometryType == "Point")
            {
                QmapPoint *myPoint = (QmapPoint *)geometry;
                if(myPoint->mylineedit != NULL)
                    delete myPoint->mylineedit;
                if(myPoint->mylabel != NULL)
                    delete myPoint->mylabel;
                disconnect(geometry);
                geometries.removeOne(geometry);
                delete geometry;


            }
        }
        //geometries.clear();
    }

    bool Layer::isVisible() const
    {
        return visible;
    }
    void Layer::zoomIn() const
    {
        mapAdapter->zoom_in();
    }
    void Layer::zoomOut() const
    {
        mapAdapter->zoom_out();
    }

    bool Layer::takesMouseEvents() const
    {
        return takeevents;
    }

    void Layer::drawYourImage(QPainter* painter, const QPoint mapmiddle_px) const
    {
        if (mylayertype == MapLayer)
        {
            //qDebug() << ":: " << mapmiddle_px;
            //QMatrix mat;
            //mat.translate(480/2, 640/2);
            //mat.rotate(45);
            //mat.translate(-480/2,-640/2);

            //mapmiddle_px = mat.map(mapmiddle_px);
            //qDebug() << ":: " << mapmiddle_px;

            _draw(painter, mapmiddle_px);

        }
        drawYourGeometries(painter, QPoint(mapmiddle_px.x()-screenmiddle.x(), mapmiddle_px.y()-screenmiddle.y()), myoffscreenViewport);
    }

    void Layer::drawYourGeometries(QPainter* painter, const QPoint mapmiddle_px, QRect viewport) const
    {
        QPoint offset;
        if (mylayertype == MapLayer)
            offset = mapmiddle_px;
        else
            offset = mapmiddle_px-screenmiddle;

        painter->translate(-mapmiddle_px+screenmiddle);
        for (int i=0; i<geometries.count(); i++)
            geometries.at(i)->draw(painter, mapAdapter, viewport, offset);

        painter->translate(mapmiddle_px-screenmiddle);

    }
    void Layer::_draw(QPainter* painter, const QPoint mapmiddle_px) const
    {
        int zoomLevel = mapadapter()->currentZoom();

        int tilesize = mapAdapter->tilesize();
        int cross_x = int(mapmiddle_px.x())%tilesize; // position on middle tile
        int cross_y = int(mapmiddle_px.y())%tilesize;
        //qDebug() << screenmiddle << " - " << cross_x << ", " << cross_y;

        // calculate how many surrounding tiles have to be drawn to fill the display
        int space_left = screenmiddle.x() - cross_x;
        int tiles_left = space_left/tilesize;
        if (space_left>0)
            tiles_left+=1;

        int space_above = screenmiddle.y() - cross_y;
        int tiles_above = space_above/tilesize;
        if (space_above>0)
            tiles_above+=1;

        int space_right = screenmiddle.x() - (tilesize-cross_x);
        int tiles_right = space_right/tilesize;
        if (space_right>0)
            tiles_right+=1;

        int space_bottom = screenmiddle.y() - (tilesize-cross_y);
        int tiles_bottom = space_bottom/tilesize;
        if (space_bottom>0)
            tiles_bottom+=1;

        //int tiles_displayed = 0;
        int mapmiddle_tile_x = mapmiddle_px.x()/tilesize;
        int mapmiddle_tile_y = mapmiddle_px.y()/tilesize;

      /*  tiles_left = 2;
        tiles_right = 2;
        tiles_above = 2;
        tiles_bottom = 2;*/

        const QPoint from = QPoint((-tiles_left+mapmiddle_tile_x)*tilesize, (-tiles_above+mapmiddle_tile_y)*tilesize);
        const QPoint to = QPoint((tiles_right+mapmiddle_tile_x+1)*tilesize, (tiles_bottom+mapmiddle_tile_y+1)*tilesize);

        myoffscreenViewport = QRect(from, to);

	// for the EmptyMapAdapter no tiles should be loaded and painted.
        if (mapAdapter->host() == "")
        {
            return;
        }

        if(zoomLevel == 20)
        {
            int x = mapmiddle_tile_x / 2;
            int y = mapmiddle_tile_y / 2;

            ImageManager::instance()->getImage(mapAdapter->host(), mapAdapter->query(x, y, 19));
            ImageManager::instance()->getImage(mapAdapter->host(), mapAdapter->query(x-1, y, 19));
            ImageManager::instance()->getImage(mapAdapter->host(), mapAdapter->query(x+1, y, 19));
            ImageManager::instance()->getImage(mapAdapter->host(), mapAdapter->query(x, y+1, 19));
            ImageManager::instance()->getImage(mapAdapter->host(), mapAdapter->query(x, y-1, 19));
        }

        if (mapAdapter->isValid(mapmiddle_tile_x, mapmiddle_tile_y, mapAdapter->currentZoom()))
        {
            painter->drawPixmap(-cross_x+size.width(),
                                -cross_y+size.height(),
                                ImageManager::instance()->getImage(mapAdapter->host(), mapAdapter->query(mapmiddle_tile_x, mapmiddle_tile_y, zoomLevel)));
        }

        for (int i=-tiles_left+mapmiddle_tile_x; i<=tiles_right+mapmiddle_tile_x; i++)
        {
            for (int j=-tiles_above+mapmiddle_tile_y; j<=tiles_bottom+mapmiddle_tile_y; j++)
            {
                // check if image is valid
                if (!(i==mapmiddle_tile_x && j==mapmiddle_tile_y))
                    if (mapAdapter->isValid(i, j, mapAdapter->currentZoom()))
                    {


                        painter->drawPixmap(((i-mapmiddle_tile_x)*tilesize)-cross_x+size.width(),
                                        ((j-mapmiddle_tile_y)*tilesize)-cross_y+size.height(),
                                        ImageManager::instance()->getImage(mapAdapter->host(), mapAdapter->query(i, j, zoomLevel)));
                    //if (QCoreApplication::hasPendingEvents())
                    //  QCoreApplication::processEvents();
                }
            }
        }
    }

    QRect Layer::offscreenViewport() const
    {
        return myoffscreenViewport;
    }

    void Layer::moveWidgets(const QPoint mapmiddle_px) const
    {
        for (int i=0; i<geometries.count(); i++)
        {
            const Geometry* geom = geometries.at(i);
            if (geom->GeometryType == "Point")
            {
                if (((QmapPoint*)geom)->widget()!=0)
                {
                    QPoint topleft_relative = QPoint(mapmiddle_px-screenmiddle);
                    ((QmapPoint*)geom)->drawWidget(mapAdapter, topleft_relative);
                }
            }
        }
    }
    Layer::LayerType Layer::layertype() const
    {
        return mylayertype;
    }

    void Layer::setMapAdapter(MapAdapter* mapadapter)
    {
        mapAdapter = mapadapter;
    }
}
