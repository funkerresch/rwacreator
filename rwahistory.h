/*
 * This file is part of the Rwa Creator.
 * An open-source cross-platform Middleware for creating interactive Soundwalks
 *
 * Copyright (C) 2015 - 2022 Thomas Resch
 *
 * License: MIT
 *
 * rwahistory.h
 * by Thomas Resch
 * Undo/History Editor
 *
 */

#ifndef RWAHISTORY_H
#define RWAHISTORY_H

#include "rwabackend.h"
#include "QListView"
#include "QFileSystemModel"
#include <QKeyEvent>
#include <QMouseEvent>

/**
 * Lists the undo snapshots (*.rwa) of the current project's undo folder.
 *
 * The view is only ever rooted at backend->completeUndoPath. When that path is
 * unset or cannot be created the view shows nothing. QFileSystemModel
 * would otherwise silently fall back to the process working directory (or the
 * drives list), and a click in there used to wipe the project.
 */
class RwaHistory : public QListView
{
    Q_OBJECT
public:
    RwaHistory(QWidget *parent);
    void keyPressEvent(QKeyEvent *event);

public slots:
    virtual void update();

    void receiveNewGameSignal();
protected:

    void mousePressEvent(QMouseEvent *event);
    /** Emits readUndoFile for the current row, if it is an undo file of the current root. */
    void loadCurrentEntry();
    RwaBackend *backend;
    QFileSystemModel *listModel;
 signals:
    void readUndoFile(QString name);

};

#endif // RWAHISTORY_H
