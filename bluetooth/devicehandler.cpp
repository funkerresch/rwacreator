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

#include <QtMath>

// Custom BLE service exposed by the RWA headtracker (RFduino/Simblee style).
static const QBluetoothUuid rwaServiceUuid(
    QStringLiteral("{713d0000-503e-4c75-ba94-3148f18d941e}"));

// Heading characteristics under that service (PROJECT-PLAN.md §5.1/§5.5):
// binary frames from rtk-rover >= 0.46.0, ASCII text from the plain RWAHT headtracker.
// Everything else on the service (e.g. the RTK raw position on 713d0004) is not
// heading data and must not reach the text parser.
static const QBluetoothUuid binaryHeadingUuid(
    QStringLiteral("{713d0005-503e-4c75-ba94-3148f18d941e}"));
static const QBluetoothUuid asciiHeadingUuid(
    QStringLiteral("{713d0002-503e-4c75-ba94-3148f18d941e}"));

namespace {

struct HeadingSample {
    float azimuth;
    float elevation;
    float linAccelZ;
};

// One binary heading frame (713d0005): 16 B little-endian,
// [seq u16][t_dev_ms u32][qi qj qk qw i16 Q14][linAccelZ i16 cm/s^2].
// seq/t_dev_ms are decoded nowhere here yet, the Creator has no
// site to show them yet.
//
// The angle math is the §5.5 canonical conversion, kept identical
// with rwa-player (Device.swift HeadingFrame). The atan2 forms are
// scale-invariant, so the quantized quaternion needs no normalization.
bool parseBinaryHeadingFrame(const QByteArray &value, HeadingSample &out)
{
    if (value.size() != 16)
        return false;

    const uchar *b = reinterpret_cast<const uchar *>(value.constData());
    auto i16 = [b](int o) {
        return qint16(quint16(b[o]) | quint16(b[o + 1]) << 8);
    };

    const float q14 = 16384.0f;
    const float qi = i16(6) / q14;
    const float qj = i16(8) / q14;
    const float qk = i16(10) / q14;
    const float qw = i16(12) / q14;

    float yaw = -std::atan2(2.0f * (qi * qj + qk * qw),
                            qi * qi - qj * qj - qk * qk + qw * qw)
                * 180.0f / float(M_PI);
    if (yaw < 0.0f)
        yaw += 360.0f;

    out.azimuth = yaw;
    out.elevation = -std::atan2(2.0f * (qj * qk + qi * qw),
                                -qi * qi - qj * qj + qk * qk + qw * qw)
                    * 180.0f / float(M_PI);
    out.linAccelZ = i16(14) / 100.0f;
    return true;
}

} // namespace

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
        m_notificationDescs.clear();  // descriptors died with the service
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
        // code subscribed to every characteristic's CCCD too. Every subscribed
        // descriptor is tracked so disconnectService() can unsubscribe them all
        // (this used to keep only the last one found).
        m_notificationDescs.clear();
        const QList<QLowEnergyCharacteristic> chars = m_service->characteristics();
        qDebug() << "[BLE debug] RWA service characteristics:" << chars.size();
        for (const QLowEnergyCharacteristic &ch : chars) {
            qDebug() << "[BLE debug]   char" << ch.uuid().toString()
                     << "properties=" << int(ch.properties());
            const QLowEnergyDescriptor cccd = ch.descriptor(
                QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
            if (cccd.isValid()) {
                m_notificationDescs.append(cccd);
                m_service->writeDescriptor(cccd, QByteArray::fromHex("0100"));
                qDebug() << "[BLE debug]   -> subscribed (wrote 0100 to CCCD)";
            }
        }

        if (m_notificationDescs.isEmpty())
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
    if (c.uuid() == binaryHeadingUuid) {
        HeadingSample sample;
        if (parseBinaryHeadingFrame(value, sample))
            emit headtrackerSampleReceived(sample.azimuth, sample.elevation,
                                           sample.linAccelZ);
        else
            qDebug() << "[BLE debug] malformed binary heading frame,"
                     << value.size() << "bytes";
        return;
    }

    if (c.uuid() == asciiHeadingUuid) {
        emit headtrackerDataReceived(QString::fromUtf8(value));
        return;
    }

    // Other subscribed characteristics (the RTK raw position on 713d0004,
    // future additions) carry no heading and are ignored here.
}

void DeviceHandler::confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value)
{
    if (d.isValid() && m_notificationDescs.contains(d)
            && value == QByteArray::fromHex("0000")) {
        //disabled notifications -> assume disconnect intent
        m_notificationDescs.removeAll(d);
        if (m_notificationDescs.isEmpty()) {
            m_control->disconnectFromDevice();
            delete m_service;
            m_service = nullptr;
        }
    }
}

void DeviceHandler::disconnectService()
{
    m_foundHeadtrackerService = false;

    //disable notifications
    bool unsubscribing = false;
    if (m_service) {
        for (const QLowEnergyDescriptor &d : std::as_const(m_notificationDescs)) {
            if (d.isValid() && d.value() == QByteArray::fromHex("0100")) {
                m_service->writeDescriptor(d, QByteArray::fromHex("0000"));
                unsubscribing = true;
            }
        }
    }
    if (!unsubscribing) {
        if (m_control)
            m_control->disconnectFromDevice();

        delete m_service;
        m_service = nullptr;
        m_notificationDescs.clear();
    }
    // else: confirmedDescriptorWrite disconnects once every 0000 is confirmed.
}

bool DeviceHandler::alive() const
{
    if (m_service)
        return m_service->state() == QLowEnergyService::RemoteServiceDiscovered;

    return false;
}
