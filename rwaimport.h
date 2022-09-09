/*
 * This file is part of the Rwa Creator.
 * An open-source cross-platform Middleware for creating interactive Soundwalks
 *
 * Copyright (C) 2015 - 2022 Thomas Resch
 *
 * License: MIT
 *
 * rwaimport.h
 * by Thomas Resch
 *
 */

#ifndef XBELREADER_H
#define XBELREADER_H

#include <QIcon>
#include <QXmlStreamReader>
#include "rwabackend.h"

class RwaImport : QObject
{

public:
    RwaImport(QObject *parent, QList<RwaScene *> *scenes, QString projectPath);
    RwaImport(QObject *parent, std::list<RwaScene *> *scenes, QString projectPath);
    RwaImport(QObject *parent, std::list<RwaScene *> &scenes, QString projectPath);

    bool read(QIODevice *device);
    QString errorString() const;

private:    
    QString readRwaInit();
    QString readinit(QIODevice *device);

    void readActions();
    void readRwa();   
    void readState();
    void readEnterconditions();
    void readAssets();
    void readRequiredStates();
    void readCorners();
    void readSceneCorners();
    void readSceneCornerLonAndLat();
    void readReflectionPositions(RwaAsset1 *asset);
    void readChannelPositions(RwaAsset1 *asset);

    RwaBackend *backend;
    QList <RwaScene *> *scenes;
    RwaScene *currentScene;
    RwaState *currentState;
    QString projectPath;
    QStringList states;
    QXmlStreamReader xml;
    QIcon folderIcon;
    QIcon bookmarkIcon;

    bool findSurroundingSceneArea = false;
    int currentStateAreaType = false;
};

#endif
