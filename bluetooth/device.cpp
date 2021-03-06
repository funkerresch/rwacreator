/***************************************************************************
**
** Copyright (C) 2013 BlackBerry Limited. All rights reserved.
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "device.h"

#include <qbluetoothaddress.h>
#include <qbluetoothdevicediscoveryagent.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothdeviceinfo.h>
#include <qbluetoothservicediscoveryagent.h>
#include <QDebug>
#include <QList>
#include <QTimer>

Device::Device():
    connected(false), controller(0), m_deviceScanState(false), randomAddress(false)
{
    discoveryAgent = new QBluetoothDeviceDiscoveryAgent();

    discoveryAgent->setLowEnergyDiscoveryTimeout(20000);

    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &Device::addDevice);
    connect(discoveryAgent, QOverload<QBluetoothDeviceDiscoveryAgent::Error>::of(&QBluetoothDeviceDiscoveryAgent::error),
            this, &Device::deviceScanError);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &Device::deviceScanFinished);
}

Device::~Device()
{
    delete discoveryAgent;
    delete controller;
    qDeleteAll(devices);
    qDeleteAll(m_services);
    qDeleteAll(m_characteristics);
    devices.clear();
    m_services.clear();
    m_characteristics.clear();
}

void Device::startDeviceDiscovery(QString name)
{
    headtrackerName = name;
    qDebug() << headtrackerName;
    qDeleteAll(devices);
    devices.clear();
    emit devicesUpdated();

    discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);

    if (discoveryAgent->isActive()) {
        m_deviceScanState = true;
        Q_EMIT stateChanged();
    }
}

void Device::addDevice(const QBluetoothDeviceInfo &info)
{
    if (info.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        DeviceInfo *d = new DeviceInfo(info);
        devices.append(d);
    }
}

void Device::deviceScanFinished()
{
    emit devicesUpdated();
    m_deviceScanState = false;
    emit stateChanged();
    if (devices.isEmpty())
        qDebug() << "Did not find RWA Headtracker";
    else
    {
        setUpdate("Done! Scan Again!");
        for (int i = 0; i < devices.size(); i++) {
            if(! (((DeviceInfo*)devices.at(i))->getName()).compare(headtrackerName) )
            {
                qDebug() << "Found RWA Headtracker";
                scanServices( ((DeviceInfo*)devices.at(i))->getAddress() );
            }
        }
    }
}

QVariant Device::getDevices()
{
    return QVariant::fromValue(devices);
}

QVariant Device::getServices()
{
    return QVariant::fromValue(m_services);
}

QVariant Device::getCharacteristics()
{
    return QVariant::fromValue(m_characteristics);
}

QString Device::getUpdate()
{
    return m_message;
}

void Device::scanServices(const QString &address)
{
    // We need the current device for service discovery.

    for (int i = 0; i < devices.size(); i++) {
        if (((DeviceInfo*)devices.at(i))->getAddress() == address )
            currentDevice.setDevice(((DeviceInfo*)devices.at(i))->getDevice());
    }

    if (!currentDevice.getDevice().isValid()) {
        qWarning() << "Not a valid device";
        return;
    }

    qDeleteAll(m_characteristics);
    m_characteristics.clear();
    emit characteristicsUpdated();
    qDeleteAll(m_services);
    m_services.clear();
    emit servicesUpdated();

    setUpdate("Back\n(Connecting to device...)");

    if (controller && m_previousAddress != currentDevice.getAddress()) {
        controller->disconnectFromDevice();
        delete controller;
        controller = 0;
    }

    if (!controller) {
        // Connecting signals and slots for connecting to LE services.
        controller = new QLowEnergyController(currentDevice.getDevice());
        connect(controller, &QLowEnergyController::connected,
                this, &Device::deviceConnected);
        connect(controller, QOverload<QLowEnergyController::Error>::of(&QLowEnergyController::error),
                this, &Device::errorReceived);
        connect(controller, &QLowEnergyController::disconnected,
                this, &Device::deviceDisconnected);
        connect(controller, &QLowEnergyController::serviceDiscovered,
                this, &Device::addLowEnergyService);
        connect(controller, &QLowEnergyController::discoveryFinished,
                this, &Device::serviceScanDone);
    }

    if (isRandomAddress())
        controller->setRemoteAddressType(QLowEnergyController::RandomAddress);
    else
        controller->setRemoteAddressType(QLowEnergyController::PublicAddress);
    controller->connectToDevice();

    m_previousAddress = currentDevice.getAddress();
}

void Device::addLowEnergyService(const QBluetoothUuid &serviceUuid)
{
    QLowEnergyService *service = controller->createServiceObject(serviceUuid);
    if (!service) {
        qWarning() << "Cannot create service for uuid";
        return;
    }

    qDebug() << "add service";

    ServiceInfo *serv = new ServiceInfo(service);
    m_services.append(serv);
    emit servicesUpdated();
}

void Device::serviceScanDone()
{
    if (m_services.isEmpty())
        emit servicesUpdated();

    connectToService(QString("713d0000-503e-4c75-ba94-3148f18d941e"));
    qDebug() << "TRYING TO CONNECT TO RWA SERVICE";
}

void Device::connectToService(const QString &uuid)
{
    QLowEnergyService *service = 0;
    for (int i = 0; i < m_services.size(); i++) {
        ServiceInfo *serviceInfo = (ServiceInfo*)m_services.at(i);
        qDebug() << serviceInfo->getUuid();
        if (serviceInfo->getUuid() == uuid) {
            service = serviceInfo->service();
            qDebug() << "connect";

            break;
        }
    }

    if (!service)
        return;

    qDeleteAll(m_characteristics);
    m_characteristics.clear();
    emit characteristicsUpdated();

    if (service->state() == QLowEnergyService::DiscoveryRequired)
    {
        connect(service, &QLowEnergyService::stateChanged,
                this, &Device::serviceDetailsDiscovered);
        service->discoverDetails();
        return;
    }

    //discovery already done
    const QList<QLowEnergyCharacteristic> chars = service->characteristics();
    foreach (const QLowEnergyCharacteristic &ch, chars) {
        CharacteristicInfo *cInfo = new CharacteristicInfo(ch);
        qDebug() << "append chars";
        m_characteristics.append(cInfo);

    }

    QTimer::singleShot(0, this, &Device::characteristicsUpdated);
}

void Device::on_characteristicChanged(QLowEnergyCharacteristic c, QByteArray a)
{
    emit sendHeadtrackerData(QString(a));
}

void Device::deviceConnected()
{
    connected = true;
    controller->discoverServices();
}

void Device::errorReceived(QLowEnergyController::Error /*error*/)
{
    qWarning() << "Error: " << controller->errorString();
}

void Device::setUpdate(QString message)
{
    m_message = message;
    emit updateChanged();
}

void Device::disconnectFromDevice()
{
    if (controller->state() != QLowEnergyController::UnconnectedState)
        controller->disconnectFromDevice();
    else
        deviceDisconnected();
}

void Device::deviceDisconnected()
{
    qWarning() << "Disconnect from device";
    emit disconnected();
}

void Device::serviceDetailsDiscovered(QLowEnergyService::ServiceState newState)
{
    if (newState != QLowEnergyService::ServiceDiscovered) {
        // do not hang in "Scanning for characteristics" mode forever
        // in case the service discovery failed
        // We have to queue the signal up to give UI time to even enter
        // the above mode
        if (newState != QLowEnergyService::DiscoveringServices) {
            QMetaObject::invokeMethod(this, "characteristicsUpdated",
                                      Qt::QueuedConnection);
        }
        return;
    }

    QLowEnergyService *service = qobject_cast<QLowEnergyService *>(sender());
    if (!service)
        return;

    const QList<QLowEnergyCharacteristic> chars = service->characteristics();
    foreach (const QLowEnergyCharacteristic &ch, chars) {
        CharacteristicInfo *cInfo = new CharacteristicInfo(ch);
        m_characteristics.append(cInfo);

        qDebug() << "check for chars";
        if(service->characteristics().isEmpty())
        {
            qDebug() << "NO CHARS";
            return;
        }
        else
            qDebug() << "Register for changes";

        QLowEnergyDescriptor notification = ch.descriptor(
            QBluetoothUuid::ClientCharacteristicConfiguration);

        if (!notification.isValid()) {
          qDebug() << "QLowEnergyDescriptor not valid";
        }

        connect(service, SIGNAL(characteristicChanged(QLowEnergyCharacteristic, QByteArray)), this, SLOT(on_characteristicChanged(QLowEnergyCharacteristic, QByteArray)));

        service->writeDescriptor(notification, QByteArray::fromHex("0100"));
    }

    emit characteristicsUpdated();
}

void Device::deviceScanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    if (error == QBluetoothDeviceDiscoveryAgent::PoweredOffError)
        qDebug() << "Turn on bluetooth";
    else if (error == QBluetoothDeviceDiscoveryAgent::InputOutputError)
        qDebug() << "Writing or reading from device failed";
    else
        qDebug() << "Unknown Error occured";

    m_deviceScanState = false;
    emit devicesUpdated();
    emit stateChanged();
}

bool Device::state()
{
    return m_deviceScanState;
}

bool Device::hasControllerError() const
{
    if (controller && controller->error() != QLowEnergyController::NoError)
        return true;
    return false;
}

bool Device::isRandomAddress() const
{
    return randomAddress;
}

void Device::setRandomAddress(bool newValue)
{
    randomAddress = newValue;
    emit randomAddressChanged();
}
