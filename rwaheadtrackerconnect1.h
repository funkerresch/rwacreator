/*
 * This file is part of the Rwa Creator.
 * An open-source cross-platform Middleware for creating interactive Soundwalks
 *
 * Copyright (C) 2015 - 2022 Thomas Resch
 *
 * License: MIT
 *
 * rwaheadtrackerconnect.h
 * by Thomas Resch
 *
 */

#ifndef RWAHEADTRACKERCONNECT_H
#define RWAHEADTRACKERCONNECT_H

#include <QObject>
#include <QRegularExpression>
#include "bluetooth/device.h"

class RwaHeadtrackerConnect1 : public QObject
{
    Q_OBJECT
public:
    explicit RwaHeadtrackerConnect1(QObject *parent = nullptr);
    static RwaHeadtrackerConnect1 *instance;
    static RwaHeadtrackerConnect1 *getInstance();
    float getAzimuth();
    QString getName();

public slots:
    void startBluetoothScanning();
    void disconnectHeadtracker();
    void setName(QString newName);
    void calibrateHeadtracker();

private:
    Device *rwaBluetooth;
    QString name = "rwaht84";
    std::vector<float> headTrackerOrientation;
    std::vector<float> headTrackerOffset;

    uint32_t calibrationCounter = 0;
    uint32_t movingAverageIndex = 0;
    float movingAverageLinAccel[100];
    float averageAccel = 0;
    bool blockSteps = false;

    void collectCalibrationData(std::vector<float> &offsetVector, std::vector<float> &receivedOrientation, uint32_t &counter);
    void calculatedOrientation(std::vector<float> &receivedOrientation);
    void detectStep(float linAccelZ);

signals:
    void sendAzimuth(float azimuth);
    void sendElevation(float elevation);
    void sendZ(float Z);
    void sendStep();

private slots:
    void receiveHeadtrackerData(const QString &data);
    void unblockSteps();
};

#endif // RWAHEADTRACKERCONNECT_H