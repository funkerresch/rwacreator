/*
 * This file is part of the Rwa Creator.
 * An open-source cross-platform Middleware for creating interactive Soundwalks
 *
 * Copyright (C) 2015 - 2022 Thomas Resch
 *
 * License: MIT
 *
 * rwamapview.h
 * by Thomas Resch
 *
 */

#ifndef RWAMAPVIEW_H
#define RWAMAPVIEW_H

#include "rwagraphicsview.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class RwaMapView : public RwaGraphicsView
{
    Q_OBJECT
public:

    explicit RwaMapView(QWidget* parent = nullptr, RwaScene *scene = nullptr, QString name = "");
    ~RwaMapView();

    void mouseDownArrow(const QMouseEvent *event, const QPointF myPoint);
    void mouseDownRubber(const QMouseEvent *event, const QPointF myPoint);
    void mouseDownMarkee(const QMouseEvent *event, const QPointF myPoint);

    float selectRectX;
    float selectRectY;

    void moveCurrentState1(const QPointF myPoint);
    void moveCurrentScene(const QPointF myPoint);
    bool mouseDownScenes(const QPointF myPoint);

    void writeSettings();
private:
    int stateIndex2Delete;

    void readSettings();
public slots:
    void receiveMouseMoveEvent(const QMouseEvent*, const QPointF);
    void receiveMouseReleaseEvent();
    void receiveMouseDownEvent(const QMouseEvent*, const QPointF);
    void receiveSelectRect(QRectF selectRect);
    void receiveSceneName(QString name);
    void receiveUpdateCurrentStateRadius();
    void setCurrentSceneWithoutRepositioning(RwaScene *scene);
    //void receiveUpdateCurrentSceneRadius();

    bool mouseDownStates(const QPointF myPoint);
    bool mouseDownEntities(const QPointF myPoint);

    void setMapCoordinates(double lon, double lat);
    void zoomIn();
    void zoomOut();
    void adaptSize(qint32 width, qint32 height);
    void moveCurrentAsset();
    void receiveUpdateCurrentSceneRadius();
    void receiveHeroPositionEdited();

protected:
     void setCurrentScene(RwaScene *scene);
     void setCurrentState(RwaState *state);
     void setCurrentState(qint32 stateNumber);
     void setCurrentAsset(RwaAsset1 *asset);

signals:
     void sendSceneName(RwaScene *scene, QString name);
     void sendMapCoordinates(double lon, double lat);
     void sendStateCoordinate(QPointF);
     void sendSelectedStates(QStringList states);
     void sendCurrentSceneWithoutRepositioning(RwaScene *scene);
};

#endif // RWAMAPVIEW_H
