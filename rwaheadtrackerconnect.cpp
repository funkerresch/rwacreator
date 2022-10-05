#include "rwaheadtrackerconnect.h"
#include <QTimer>


RwaHeadtrackerConnect *RwaHeadtrackerConnect::instance = nullptr;

RwaHeadtrackerConnect *RwaHeadtrackerConnect::getInstance()
{
    if(RwaHeadtrackerConnect::instance == nullptr)
    {
        RwaHeadtrackerConnect::instance = new RwaHeadtrackerConnect();
    }

    return RwaHeadtrackerConnect::instance;
}

float RwaHeadtrackerConnect::getAzimuth()
{
    return headTrackerOrientation[0];
}

RwaHeadtrackerConnect::RwaHeadtrackerConnect(QObject *parent) : QObject(parent)
{   

     // deviceHandler = new DeviceHandler();
      rwaBluetooth = new Device();

    headTrackerOrientation = std::vector<float>(3, 0.0);
    headTrackerOffset = std::vector<float>(3, 0.0);
}

void RwaHeadtrackerConnect::startBluetoothScanning()
{
    qDebug() << "Start Headtracker Discovery";
    //rwaBluetooth->startDeviceDiscovery(name);
    rwaBluetooth->startDeviceDiscovery(name);
    connect (rwaBluetooth, SIGNAL(sendHeadtrackerData(QString)),
             this, SLOT(receiveHeadtrackerData(QString)));
}

void RwaHeadtrackerConnect::disconnectHeadtracker()
{
    rwaBluetooth->disconnectFromDevice();
}

void RwaHeadtrackerConnect::setName(QString newName)
{
    name = newName;
}

QString RwaHeadtrackerConnect::getName()
{
    return name;
}

void RwaHeadtrackerConnect::calibrateHeadtracker()
{
    calibrationCounter = 1;
}

void RwaHeadtrackerConnect::collectCalibrationData(std::vector<float> &offsetVector, std::vector<float> &receivedOrientation, uint32_t &counter)
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

void RwaHeadtrackerConnect::calculatedOrientation(std::vector<float> &receivedOrientation)
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

void RwaHeadtrackerConnect::unblockSteps()
{
    blockSteps = false;
}

void RwaHeadtrackerConnect::detectStep(float linAccelZ)
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

void RwaHeadtrackerConnect::receiveHeadtrackerData(const QString &data)
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

    //qDebug() << receivedOrientation[0];

    if(list.length() >= 2)
       receivedOrientation[1] = list.at(1).toFloat();

    if(list.length() >= 3)
        detectStep(list.at(2).toFloat());

    if(calibrationCounter)
        collectCalibrationData(headTrackerOffset, receivedOrientation, calibrationCounter);
    else
        calculatedOrientation(receivedOrientation);
}
