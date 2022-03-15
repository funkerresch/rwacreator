#ifndef RWAHEADTRACKERCONNECT_H
#define RWAHEADTRACKERCONNECT_H

#include <QObject>
#include "bluetooth/device.h"

class RwaHeadtrackerConnect : public QObject
{
    Q_OBJECT
public:
    explicit RwaHeadtrackerConnect(QObject *parent = nullptr);
    static RwaHeadtrackerConnect *instance;
    static RwaHeadtrackerConnect *getInstance();
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
