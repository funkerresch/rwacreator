/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "rwasuggestplaces.h"

//http://nominatim.openstreetmap.org/?format=json&addressdetails=1&q=[monument]+berlin+wedding&format=json&limit=10
//http://nominatim.openstreetmap.org/?format=xml&addressdetails=1&q=[church]+berlin+mitte&format=xml&limit=10
//http://nominatim.openstreetmap.org/search?q=[church]&format=xml&limit=10&viewbox=7.98435,49.40889,8.95440,48.77371&bounded=1

#define GSUGGEST_URL "https://nominatim.openstreetmap.org/search?q=%1&format=xml"

RwaSuggestPlaces::RwaSuggestPlaces(QLineEdit *parent): QObject(parent), editor(parent)
{
    popup = new QTreeWidget;
    popup->setWindowFlags(Qt::Popup);
    popup->setFocusPolicy(Qt::NoFocus);
    popup->setFocusProxy(parent);
    popup->setMouseTracking(true);
    popup->setColumnCount(3);
    popup->setUniformRowHeights(true);
    popup->setRootIsDecorated(false);
    popup->setEditTriggers(QTreeWidget::NoEditTriggers);
    popup->setSelectionBehavior(QTreeWidget::SelectRows);
    popup->setFrameStyle(QFrame::Box | QFrame::Plain);
    popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    popup->header()->hide();
    popup->installEventFilter(this);

    connect(popup, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            SLOT(doneCompletion()));

    timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(500);
    connect(timer, SIGNAL(timeout()), SLOT(autoSuggest()));
    connect(editor, SIGNAL(textEdited(QString)), timer, SLOT(start()));

    connect(&networkManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(handleNetworkData(QNetworkReply*)));
}

RwaSuggestPlaces::~RwaSuggestPlaces()
{
    delete popup;
}

bool RwaSuggestPlaces::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj != popup)
        return false;

    if (ev->type() == QEvent::MouseButtonPress) {
        popup->hide();
        editor->setFocus();
        return true;
    }

    if (ev->type() == QEvent::KeyPress) {

        bool consumed = false;
        int key = static_cast<QKeyEvent*>(ev)->key();
        switch (key) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            doneCompletion();
            consumed = true;
            break;

        case Qt::Key_Escape:
            editor->setFocus();
            popup->hide();
            consumed = true;
            break;

        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Home:
        case Qt::Key_End:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
            break;

        default:
            editor->setFocus();
            editor->event(ev);
            popup->hide();
            break;
        }

        return consumed;
    }

    return false;
}

void RwaSuggestPlaces::showCompletion(const QStringList &choices, const QStringList &lat, const QStringList &lon)
{
    if (choices.isEmpty())
        return;

    const QPalette &pal = editor->palette();
    QColor color = pal.color(QPalette::Disabled, QPalette::WindowText);

    popup->setUpdatesEnabled(false);
    popup->setFixedWidth(400);
    popup->clear();

    for (int i = 0; i < choices.count(); ++i)
    {
        QTreeWidgetItem * item;
        item = new QTreeWidgetItem(popup);
        item->setText(0, choices[i]);
        item->setText(1, lon[i]);
        item->setText(2, lat[i]);
    }

    popup->setCurrentItem(popup->topLevelItem(0));
    popup->resizeColumnToContents(0);
    popup->resizeColumnToContents(1);
    popup->resizeColumnToContents(2);
    popup->adjustSize();
    popup->setUpdatesEnabled(true);
    popup->move(editor->mapToGlobal(QPoint(0, editor->height())));
    popup->setFocus();
    popup->show();
}

void RwaSuggestPlaces::doneCompletion()
{
    timer->stop();
    popup->hide();
    editor->setFocus();
    double lon;
    double lat;

    QTreeWidgetItem *item = popup->currentItem();
    if (item) {
        editor->setText(item->text(0));
        QMetaObject::invokeMethod(editor, "returnPressed");
        lon = item->text(1).toDouble();
        lat = item->text(2).toDouble();
        emit sendMapCoordinates(lon, lat);
    }
}

void RwaSuggestPlaces::autoSuggest()
{
    QString str = editor->text();
    QString url = QString(GSUGGEST_URL).arg(str);
    networkManager.get(QNetworkRequest(QString(url)));
}

void RwaSuggestPlaces::preventSuggest()
{
    timer->stop();
}

void RwaSuggestPlaces::handleNetworkData(QNetworkReply *networkReply)
{
    QUrl url = networkReply->url();

    if (!networkReply->error())
    {     
        QStringList choices;
        QStringList lon;
        QStringList lat;

        QByteArray response(networkReply->readAll());
        QXmlStreamReader xml(response);
        //qDebug() << QString(response);
        while (!xml.atEnd())
        {
            xml.readNext();
            if (xml.tokenType() == QXmlStreamReader::StartElement)
            {
                if (xml.name().toString() == "place")
                {
                    auto str = xml.attributes().value("display_name");
                    choices << str.toString();

                    auto latitude = xml.attributes().value("lat");
                    lat << latitude.toString();

                    auto longitude = xml.attributes().value("lon");
                    lon << longitude.toString();
                }
            }
        }

        showCompletion(choices, lat, lon);
    }
    else
    {
        qDebug("network Error");
    }

    networkReply->deleteLater();
}


