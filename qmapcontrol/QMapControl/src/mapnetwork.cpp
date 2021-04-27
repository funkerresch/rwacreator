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

#include "mapnetwork.h"
#include <QWaitCondition>
namespace qmapcontrol
{
    MapNetwork::MapNetwork(ImageManager* parent)
        :parent(parent),  loaded(0)
    {
        httpManager = new QNetworkAccessManager(this);
        counter = 0;

        connect(httpManager,SIGNAL(finished(QNetworkReply*)) ,this,SLOT(onFinished(QNetworkReply*)));
    }

    MapNetwork::~MapNetwork()
    {
        //http->clearPendingRequests();
        //delete http;

    }

    void MapNetwork::loadImage(const QString& host, const QString& url)
    {
        QNetworkReply *reply;
        QNetworkRequest request;
        QString completeUrl;

        request.setUrl(QUrl("http://"+host+url));
        request.setRawHeader("User-Agent", "MyOwnBrowser 1.0");
        //httpManager->connectToHost(host, 80);


       // if (vectorMutex.tryLock())
        {
            completeUrl = "http://"+host+url;
            if(!loadingMap.contains(completeUrl))
            {
                reply = httpManager->get(request);
                loadingMap << completeUrl;
                // qDebug() << completeUrl;
            }
            // vectorMutex.unlock();
        }


    }

    void MapNetwork::onFinished(QNetworkReply *reply)
    {
       // reply->id();
        QByteArray ax = reply->readAll();
        //qDebug() << reply->errorString();
        QString urlWithoutHost = reply->url().toString().replace("http://tile.openstreetmap.org", "");
        QPixmap pm;

       // if (vectorMutex.tryLock())
        {
            if(loadingMap.contains(reply->url().toString()))
            {
              //  qDebug() << "onFinished: " + reply->url().toString();
                loadingMap.removeOne(reply->url().toString());
              //  vectorMutex.unlock();

                if (pm.loadFromData(ax))
                {
                    loaded += pm.size().width()*pm.size().height()*pm.depth()/8/1024;
                   // qDebug() << "Network loaded: " << (loaded);
                    parent->receivedImage(pm, urlWithoutHost);
                }
                else
                {
                    //qDebug() << "NETWORK_PIXMAP_ERROR: " << reply->url().toString();
                }
            }
            else;
               // vectorMutex.unlock();

        }
        if (loadingMap.size() == 0)
        {
            // qDebug () << "all loaded";
            parent->loadingQueueEmpty();

        }
        reply->deleteLater();

    }



    void MapNetwork::abortLoading()
    {
    //http->clearPendingRequests();

        //if (vectorMutex.tryLock())
        {
            loadingMap.clear();
           // vectorMutex.unlock();
        }
    }

    bool MapNetwork::imageIsLoading(QString url)
    {
        //return loadingMap.values().contains(url);
        QString completeURL;
        bool isLoading;
        completeURL = "http://tile.openstreetmap.org"+url;

        isLoading = loadingMap.contains(completeURL);
        //if(isLoading)
           // qDebug() << "already Loading? " + completeURL;
        return isLoading;


    }

    void MapNetwork::setProxy(QString host, int port)
    {
#ifndef Q_WS_QWS
        // do not set proxy on qt/extended
        //http->setProxy(host, port);
#endif
    }
}
