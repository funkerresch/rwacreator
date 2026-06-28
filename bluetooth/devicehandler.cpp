/***************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
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
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES IS DISCLAIMED.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "devicehandler.h"
#include "deviceinfo.h"

// Custom BLE service exposed by the RWA headtracker (RFduino/Simblee style).
// Its notify characteristic streams orientation as an ASCII text payload.
static const QBluetoothUuid rwaServiceUuid(
    QStringLiteral("{713d0000-503e-4c75-ba94-3148f18d941e}"));

DeviceHandler::DeviceHandler(QObject *parent) :
    BluetoothBaseClass(parent)
{
}

void DeviceHandler::setAddressType(AddressType type)
{
    switch (type) {
    case DeviceHandler::AddressType::PublicAddress:
        m_addressType = QLowEnergyController::PublicAddress;
        break;
    case DeviceHandler::AddressType::RandomAddress:
        m_addressType = QLowEnergyController::RandomAddress;
        break;
    }
}

DeviceHandler::AddressType DeviceHandler::addressType() const
{
    if (m_addressType == QLowEnergyController::RandomAddress)
        return DeviceHandler::AddressType::RandomAddress;

    return DeviceHandler::AddressType::PublicAddress;
}

void DeviceHandler::setDevice(DeviceInfo *device)
{
    clearMessages();
    m_currentDevice = device;

    // Disconnect and delete old connection
    if (m_control) {
        m_control->disconnectFromDevice();
        delete m_control;
        m_control = nullptr;
    }

    // Create new controller and connect it if device available
    if (m_currentDevice) {
        m_control = QLowEnergyController::createCentral(m_currentDevice->getDevice(), this);
        m_control->setRemoteAddressType(m_addressType);
        connect(m_control, &QLowEnergyController::serviceDiscovered,
                this, &DeviceHandler::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished,
                this, &DeviceHandler::serviceScanDone);

        connect(m_control, &QLowEnergyController::errorOccurred, this,
                [this](QLowEnergyController::Error error) {
                    Q_UNUSED(error);
                    setError("Cannot connect to remote device.");
                });
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            setInfo("Controller connected. Search services...");
            m_control->discoverServices();
        });
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            setError("LowEnergy controller disconnected");
        });

        // Connect
        m_control->connectToDevice();
    }
}

void DeviceHandler::serviceDiscovered(const QBluetoothUuid &gatt)
{
    if (gatt == rwaServiceUuid) {
        setInfo("RWA headtracker service discovered. Waiting for service scan to be done...");
        m_foundHeadtrackerService = true;
    }
}

void DeviceHandler::serviceScanDone()
{
    setInfo("Service scan done.");

    // Delete old service if available
    if (m_service) {
        delete m_service;
        m_service = nullptr;
    }

    // If the headtracker service was found, create the service object
    if (m_foundHeadtrackerService)
        m_service = m_control->createServiceObject(rwaServiceUuid, this);

    if (m_service) {
        connect(m_service, &QLowEnergyService::stateChanged, this, &DeviceHandler::serviceStateChanged);
        connect(m_service, &QLowEnergyService::characteristicChanged, this, &DeviceHandler::handleCharacteristicData);
        connect(m_service, &QLowEnergyService::descriptorWritten, this, &DeviceHandler::confirmedDescriptorWrite);
        m_service->discoverDetails();
    } else {
        setError("RWA headtracker service not found.");
    }
}

void DeviceHandler::serviceStateChanged(QLowEnergyService::ServiceState s)
{
    switch (s) {
    case QLowEnergyService::RemoteServiceDiscovering:
        setInfo(tr("Discovering services..."));
        break;
    case QLowEnergyService::RemoteServiceDiscovered:
    {
        setInfo(tr("Service discovered."));

        // subscribe by writing 0100 to the CCCD of every characteristic that has
        // one. deliberately do NOT filter on the Notify property flag - the
        // RWA headtracker firmware doesn't always report it, and the old working
        // code subscribed to every characteristic's CCCD too.
        const QList<QLowEnergyCharacteristic> chars = m_service->characteristics();
        qDebug() << "[BLE debug] RWA service characteristics:" << chars.size();
        for (const QLowEnergyCharacteristic &ch : chars) {
            qDebug() << "[BLE debug]   char" << ch.uuid().toString()
                     << "properties=" << int(ch.properties());
            const QLowEnergyDescriptor cccd = ch.descriptor(
                QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
            if (cccd.isValid()) {
                m_notificationDesc = cccd;
                m_service->writeDescriptor(cccd, QByteArray::fromHex("0100"));
                qDebug() << "[BLE debug]   -> subscribed (wrote 0100 to CCCD)";
            }
        }

        if (!m_notificationDesc.isValid())
            setError("No characteristic with a CCCD found on RWA service.");

        break;
    }
    default:
        //nothing for now
        break;
    }

    emit aliveChanged();
}

void DeviceHandler::handleCharacteristicData(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    Q_UNUSED(c);
    emit headtrackerDataReceived(QString::fromUtf8(value));
}

void DeviceHandler::confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value)
{
    if (d.isValid() && d == m_notificationDesc && value == QByteArray::fromHex("0000")) {
        //disabled notifications -> assume disconnect intent
        m_control->disconnectFromDevice();
        delete m_service;
        m_service = nullptr;
    }
}

void DeviceHandler::disconnectService()
{
    m_foundHeadtrackerService = false;

    //disable notifications
    if (m_notificationDesc.isValid() && m_service
            && m_notificationDesc.value() == QByteArray::fromHex("0100")) {
        m_service->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0000"));
    } else {
        if (m_control)
            m_control->disconnectFromDevice();

        delete m_service;
        m_service = nullptr;
    }
}

bool DeviceHandler::alive() const
{
    if (m_service)
        return m_service->state() == QLowEnergyService::RemoteServiceDiscovered;

    return false;
}
