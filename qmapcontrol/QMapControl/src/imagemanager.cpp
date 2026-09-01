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

#include "imagemanager.h"
#include "mapnetwork.h"
namespace qmapcontrol
{
    ImageManager* ImageManager::m_Instance = 0;
    ImageManager::ImageManager(QObject* parent)
        :QObject(parent), emptyPixmap(QPixmap(1,1)), net(new MapNetwork(this)), doPersistentCaching(true)
    {
        emptyPixmap.fill(Qt::transparent);

       // if (QPixmapCache::cacheLimit() <= 80000)
        {
            QPixmapCache::setCacheLimit(50000);	// in kb
        }
    }


    ImageManager::~ImageManager()
    {
	if (ImageManager::m_Instance != 0)
	{
	    delete ImageManager::m_Instance;
	}
        delete net;
    }

    void ImageManager::saveFilesForZoomLevel20(QPixmap pm, int x, int y)
    {
        QString url_nw = QString("/%1/%2/%3.png").arg(20).arg(2*x).arg(2*y);
        QString url_ne = QString("/%1/%2/%3.png").arg(20).arg(2*x+1).arg(2*y);
        QString url_sw = QString("/%1/%2/%3.png").arg(20).arg(2*x).arg(2*y+1);
        QString url_se = QString("/%1/%2/%3.png").arg(20).arg(2*x+1).arg(2*y+1);

        if(!tileExist(url_nw))
        {
            QPixmap nw = pm.copy(0,0,128, 128);
            saveTile(url_nw, nw.scaled(256, 256));
        }

        if(!tileExist(url_ne))
        {
            QPixmap ne = pm.copy(128,0,128, 128);
            saveTile(url_ne, ne.scaled(256, 256));
        }

        if(!tileExist(url_sw))
        {
            QPixmap sw = pm.copy(0,128,128, 128);
            saveTile(url_sw, sw.scaled(256, 256));
        }

        if(!tileExist(url_se))
        {
            QPixmap se = pm.copy(128,128,128, 128);
            saveTile(url_se, se.scaled(256, 256));
        }
    }

    void ImageManager::getImageForZoom20(const QString& host, const QString& url, int x, int y)
    {
        QPixmap pm;

        QString urlCopy = url;
        QString url_nw = QString("/%1/%2/%3.png").arg(20).arg(2*x).arg(2*y);
        QString url_ne = QString("/%1/%2/%3.png").arg(20).arg(2*x+1).arg(2*y);
        QString url_sw = QString("/%1/%2/%3.png").arg(20).arg(2*x).arg(2*y+1);
        QString url_se = QString("/%1/%2/%3.png").arg(20).arg(2*x+1).arg(2*y+1);

        if(tileExist(url))
        {
            loadTile(url,pm);
            QPixmap nw = pm.copy(0,0,128, 128);
            QPixmap ne = pm.copy(128,0,128, 128);
            QPixmap sw = pm.copy(0,128,128, 128);
            QPixmap se = pm.copy(128,128,128, 128);
            saveTile(url_nw, nw.scaled(256, 256));
            saveTile(url_ne, ne.scaled(256, 256));
            saveTile(url_sw, sw.scaled(256, 256));
            saveTile(url_se, se.scaled(256, 256));
        }
    }

    QPixmap ImageManager::getImage(const QString& host, const QString& url)
    {
        QPixmap pm;
        QString urlCopy = url;
        QStringList tmp = url.split("/", Qt::SkipEmptyParts );
        QStringList tmp2 = tmp[2].split(".", Qt::SkipEmptyParts );
        QString zoom = tmp[0];
        int x = tmp[1].toInt();
        int y = tmp2[0].toInt();

        if (doPersistentCaching && tileExist(urlCopy))
        {
           // qDebug() << "found image in cache";
            loadTile(urlCopy,pm);
            saveFilesForZoomLevel20(pm, x, y);


           // QPixmapCache::insert(url.toLatin1().toBase64(), pm);
            return pm;
        }

        else
        {
            if(!net->imageIsLoading(urlCopy))
            {
                net->loadImage(host, urlCopy);
                //QPixmapCache::insert(url, emptyPixmap);
                 return emptyPixmap;
            }
        }

        return emptyPixmap;
    }

    QPixmap ImageManager::prefetchImage(const QString& host, const QString& url)
    {
#ifdef Q_WS_QWS
        // on mobile devices we don´t want the display resfreshing when tiles are received which are
        // prefetched... This is a performance issue, because mobile devices are very slow in
        // repainting the screen
        prefetch.append(url);
#endif
        return getImage(host, url);
    }

    void ImageManager::receivedImage(const QPixmap pixmap, const QString& url)
    {
        QPixmapCache::insert(url, pixmap);
        QStringList tmp = url.split("/", Qt::SkipEmptyParts );
        QStringList tmp2 = tmp[2].split(".", Qt::SkipEmptyParts );
        QString zoom = tmp[0];
        int x = tmp[1].toInt();
        int y = tmp2[0].toInt();

        // needed?
        if (doPersistentCaching && !tileExist(url) )
        {
            saveTile(url,pixmap);
            saveFilesForZoomLevel20(pixmap, x, y);
        }

        //((Layer*)this->parent())->imageReceived();

        if (!prefetch.contains(url))
        {
            emit imageReceived();
        }
        else
        {

#ifdef Q_WS_QWS
            prefetch.remove(prefetch.indexOf(url));
#endif
        }
    }

    void ImageManager::loadingQueueEmpty()
    {
        emit loadingFinished();
    }

    void ImageManager::abortLoading()
    {
        net->abortLoading();
    }
    void ImageManager::setProxy(QString host, int port)
    {
        net->setProxy(host, port);
    }

    void ImageManager::setCacheDir(const QDir& path)
    {
        doPersistentCaching = true;
           //qDebug() << "Set Cache Dir to: " << path;
        cacheDir = path;
        if (!cacheDir.exists())
        {
            cacheDir.mkpath(cacheDir.absolutePath());
        }
    }

    bool ImageManager::saveTile(QString tileName,QPixmap tileData)
    {
        tileName.replace("/","-");

        QFile file(cacheDir.absolutePath() + "/" + tileName.toLatin1().toBase64());

        //qDebug() << "writing: " << file.fileName();
        if (!file.open(QIODevice::ReadWrite )){
            qDebug()<<"error reading file";
            return false;
        }
        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        tileData.save(&buffer, "PNG");

        file.write(bytes);
        file.close();
        return true;
    }
    bool ImageManager::loadTile(QString tileName,QPixmap &tileData)
    {
        tileName.replace("/","-");
        QFile file(cacheDir.absolutePath() + "/" + tileName.toLatin1().toBase64());
        if (!file.open(QIODevice::ReadOnly )) {
            return false;
        }
        tileData.loadFromData( file.readAll() );

        file.close();
        return true;
    }
    bool ImageManager::tileExist(QString tileName)
    {
        tileName.replace("/","-");
        QFile file(cacheDir.absolutePath() + "/" + tileName.toLatin1().toBase64());
        if (file.exists())
            return true;
        else
            return false;
    }
}
