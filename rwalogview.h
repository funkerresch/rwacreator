/*
 * This file is part of the Rwa Creator.
 * An open-source cross-platform Middleware for creating interactive Soundwalks
 *
 * Copyright (C) 2015 - 2022 Thomas Resch
 *
 * License: MIT
 *
 * rwalogview.h
 * by Thomas Resch
 * Logging window
 *
 */

#ifndef RWALOGWRAPPER_H
#define RWALOGWRAPPER_H


#include <QWidget>
#include <QTextBrowser>
#include <QPushButton>
#include <QCheckBox>
#include "rwabackend.h"


class RwaLogWindow : public QWidget
{
 Q_OBJECT
public:
 explicit RwaLogWindow(QWidget *parent = nullptr);
 ~RwaLogWindow();

    RwaBackend *backend;
    QTextBrowser *browser;
    QPushButton *clearButton;
    QCheckBox *logLongAndLatCheckbox;
    QCheckBox *logLibPdPrint;
    QCheckBox *logSimulatorStates;
    QCheckBox *logOther;

public slots:
    void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg );
};

#endif // RWALOGWRAPPER_H
