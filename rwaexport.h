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
#include "rwabackend.h"

#define RWAEXPORT_COPYASSETS 1 << 0
#define RWAEXPORT_SAVEAS 1 << 1
#define RWAEXPORT_SAVEACOPYIN 1 << 2
#define RWAEXPORT_EXPORTFORMOBILECLIENT 1 << 3
#define RWAEXPORT_SAVE 1 << 4
#define RWAEXPORT_ZIP 1 << 5
#define RWAEXPORT_CREATEFOLDERS 1 << 6
#define RWAEXPORT_CONTENTONLY 1 << 7   // omit view state (selection, map zoom); used for the modified-check

class RwaExport : QObject
{
public:
    RwaExport(QObject *parent, QString originalProjectPath, QString newProjectPath, qint32 flags);

    /**
     * Writes the game as .rwa XML to any QIODevice: the project file, an undo
     * snapshot or, with RWAEXPORT_CONTENTONLY, an in-memory buffer whose
     * bytes only depend on the authored content, so that two exports compare
     * equal exactly when nothing worth saving has changed.
     */
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
    bool contentOnly;
    int flags;
};

#endif
