#include "serialportthread.h"
#include <QtSerialPort/QSerialPort>
#include <QTime>

QT_USE_NAMESPACE

SlaveThread::SlaveThread(QObject *parent)
    : QThread(parent), waitTimeout(0), quit(false)
{
}

SlaveThread::~SlaveThread()
{
    mutex.lock();
    quit = true;
    mutex.unlock();
    wait();
}

void SlaveThread::startSlave(const QString &portName, int waitTimeout, const QString &response)
{
    QMutexLocker locker(&mutex);
    this->portName = portName;
    this->waitTimeout = waitTimeout;
    this->response = response;

    if (!isRunning())
        start();
}

void SlaveThread::run()
{
    bool currentPortNameChanged = false;

    mutex.lock();

    QString currentPortName;
    if (currentPortName != portName) {
        currentPortName = portName;
        currentPortNameChanged = true;
    }

    int currentWaitTimeout = waitTimeout;
    QString currentRespone = response;
    mutex.unlock();
    QSerialPort serial;

    while (!quit)
    {
        if (currentPortNameChanged)
        {
            serial.close();
            serial.setPortName(currentPortName);
            serial.setBaudRate(QSerialPort::Baud9600);

            if (!serial.open(QIODevice::ReadWrite))
            {
                emit error(tr("Can't open %1, error code %2")
                           .arg(portName).arg(serial.error()));
                return;
            }
        }

        if (serial.waitForReadyRead(currentWaitTimeout))
        {
            QByteArray requestData = serial.readAll();
            while (serial.waitForReadyRead(25))
                requestData += serial.readAll();

            // write response
            QByteArray responseData = currentRespone.toLocal8Bit();
            serial.write(responseData);
           // if (serial.waitForBytesWritten(waitTimeout))
            {
                QString request(requestData);
                //qDebug(request.toLatin1());
                emit this->request(request);
                //serial.flush();

            } //else {
            //    qDebug("TIMEOUT 1");
                //emit timeout(tr("Wait write response timeout %1")
                          //   .arg(QTime::currentTime().toString()));
          //  }

        } else {
            //qDebug("TIMEOUT 2");
           // emit timeout(tr("Wait read request timeout %1")
                        // .arg(QTime::currentTime().toString()));
        }

        mutex.lock();
        if (currentPortName != portName) {
            currentPortName = portName;
            currentPortNameChanged = true;
        } else {
            currentPortNameChanged = false;
        }
        currentWaitTimeout = waitTimeout;
        currentRespone = response;
        mutex.unlock();
        serial.flush();
    }
}
