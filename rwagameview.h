/*
 * This file is part of the Rwa Creator.
 * An open-source cross-platform Middleware for creating interactive Soundwalks
 *
 * Copyright (C) 2015 - 2022 Thomas Resch
 *
 * License: MIT
 *
 * rwagameview.h
 * by Thomas Resch
 *
 */

#ifndef RWAGAMEVIEW_H
#define RWAGAMEVIEW_H

#include "rwaview.h"
#include "rwascenelist.h"
#include "rwasceneattributeview.h"

class RwaGameView : public RwaView
{
    Q_OBJECT
public:
    explicit RwaGameView(QWidget *parent = nullptr, RwaScene *scene = nullptr, QString name = "");

//public slots:
//    void adaptSize(qint32 width, qint32 height);

private:

    RwaSceneList *sceneList;
    RwaSceneAttributeView *sceneAttributes;

signals:
    void updateAttributes();

};

#endif // RWASTATEVIEW_H
