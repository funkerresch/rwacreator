#include "rwaheadtrackerconnect1.h"
#include <QTimer>

RwaHeadtrackerConnect1 *RwaHeadtrackerConnect1::instance = nullptr;

RwaHeadtrackerConnect1 *RwaHeadtrackerConnect1::getInstance()
{
    if(RwaHeadtrackerConnect1::instance == nullptr)
    {
        RwaHeadtrackerConnect1::instance = new RwaHeadtrackerConnect1();
    }

    return RwaHeadtrackerConnect1::instance;
}

float RwaHeadtrackerConnect1::getAzimuth()
{
    return headTrackerOrientation[0];
}

RwaHeadtrackerConnect1::RwaHeadtrackerConnect1(QObject *parent) : QObject(parent)
{   
    rwaBluetooth = new Device();
    headTrackerOrientation = std::vector<float>(3, 0.0);
    headTrackerOffset = std::vector<float>(3, 0.0);
}

void RwaHeadtrackerConnect1::startBluetoothScanning()
{
    qDebug() << "Start Headtracker Discovery";
    rwaBluetooth->startDeviceDiscovery(name);
    connect (rwaBluetooth, SIGNAL(sendHeadtrackerData(QString)),
             this, SLOT(receiveHeadtrackerData(QString)));
}

void RwaHeadtrackerConnect1::disconnectHeadtracker()
{
    rwaBluetooth->disconnectFromDevice();
}

void RwaHeadtrackerConnect1::setName(QString newName)
{
    name = newName;
}

QString RwaHeadtrackerConnect1::getName()
{
    return name;
}

void RwaHeadtrackerConnect1::calibrateHeadtracker()
{
    calibrationCounter = 1;
}

void RwaHeadtrackerConnect1::collectCalibrationData(std::vector<float> &offsetVector, std::vector<float> &receivedOrientation, uint32_t &counter)
{
    if(counter <= 100)
    {
        offsetVector[0]+=receivedOrientation[0];
        offsetVector[1]+=receivedOrientation[1];
        offsetVector[2]+=receivedOrientation[2];
        counter++;
    }
    else
    {
        offsetVector[0]/=100;
        offsetVector[1]/=100;
        offsetVector[2]/=100;
        counter = 0;
        qDebug() << "Done Calibrating";
    }
}

void RwaHeadtrackerConnect1::calculatedOrientation(std::vector<float> &receivedOrientation)
{
    headTrackerOrientation[0] = receivedOrientation[0] - headTrackerOffset[0];
    if(headTrackerOrientation[0] < 0)
        headTrackerOrientation[0]+=360;

    headTrackerOrientation[1] = receivedOrientation[1] - headTrackerOffset[1];
    headTrackerOrientation[2] = receivedOrientation[2] - headTrackerOffset[2];

    emit sendAzimuth(headTrackerOrientation[0]);
    emit sendElevation(headTrackerOrientation[1]);
    emit sendZ(headTrackerOrientation[2]);
}

void RwaHeadtrackerConnect1::unblockSteps()
{
    blockSteps = false;
}

void RwaHeadtrackerConnect1::detectStep(float linAccelZ)
{
    movingAverageLinAccel[movingAverageIndex] = linAccelZ;
    movingAverageIndex++;
    if(movingAverageIndex >= 100)
         movingAverageIndex = 0;

    averageAccel = 0;

    for(int i=0;i<100;i++)
        averageAccel += movingAverageLinAccel[i];

    averageAccel /= 100;

    if(!blockSteps)
    {
        if(linAccelZ >= 0)
        {
             if(averageAccel >= 0)
             {
                 float dif = linAccelZ-averageAccel;
                 if(dif > 0.6f)
                 {
                     emit sendStep();
                     QTimer::singleShot(500, this, SLOT(unblockSteps()));
                     blockSteps = true;
                 }
             }
             else
             {
                 float dif = linAccelZ+averageAccel;
                 if(dif > 0.6f)
                 {
                     emit sendStep();
                     QTimer::singleShot(500, this, SLOT(unblockSteps()));
                     blockSteps = true;
                 }
             }
         }
     }
 }

void RwaHeadtrackerConnect1::receiveHeadtrackerData(const QString &data)
{
    std::vector<float> receivedOrientation = std::vector<float>(3, 0.0);

    if(data.isEmpty())
        return;

   // QStringList list = data.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
    QStringList list = data.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    if(list.empty())
        return;

    if(list.length() >= 1)
        receivedOrientation[0] = list.at(0).toFloat();

    qDebug() << receivedOrientation[0];

    if(list.length() >= 2)
       receivedOrientation[1] = list.at(1).toFloat();

    if(list.length() >= 3)
        detectStep(list.at(2).toFloat());

    if(calibrationCounter)
        collectCalibrationData(headTrackerOffset, receivedOrientation, calibrationCounter);
    else
        calculatedOrientation(receivedOrientation);
}
