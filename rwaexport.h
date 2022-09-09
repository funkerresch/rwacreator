/*
 * This file is part of the Rwa Creator.
 * An open-source cross-platform Middleware for creating interactive Soundwalks
 *
 * Copyright (C) 2015 - 2022 Thomas Resch
 *
 * License: MIT
 *
 * rwaexport.h
 * by Thomas Resch
 *
 */

#ifndef XBELWRITER_H
#define XBELWRITER_H

#include <QXmlStreamWriter>
#include <rwabackend.h>

#define RWAEXPORT_COPYASSETS 1 << 0
#define RWAEXPORT_SAVEAS 1 << 1
#define RWAEXPORT_SAVEACOPYIN 1 << 2
#define RWAEXPORT_EXPORTFORMOBILECLIENT 1 << 3
#define RWAEXPORT_SAVE 1 << 4
#define RWAEXPORT_ZIP 1 << 5
#define RWAEXPORT_CREATEFOLDERS 1 << 6

class RwaExport : QObject
{
public:
    RwaExport(QObject *parent, QString originalProjectPath, QString newProjectPath, qint32 flags);
    bool writeFile(QIODevice *device);

private:
    void writeScene(RwaScene *scene);
    void writeState(RwaState *state);
    void writeAssetItem1(RwaAsset1 *item);

    QXmlStreamWriter xml;
    RwaBackend *backend;
    QString path;
    QString newProjectPath;
    bool copyAssets;
    int flags;
};

#endif
