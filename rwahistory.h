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
#include "QStringListModel"

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
    RwaBackend *backend;
    QFileSystemModel *listModel;
 signals:
    void readUndoFile(QString name);

};

#endif // RWAHISTORY_H
