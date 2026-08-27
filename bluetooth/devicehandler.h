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

// Adapted from the Qt "Heart Rate Game" example for the RWA headtracker:
// connects to the RWA BLE service (713d0000) and forwards heading data,
// dispatched per characteristic:
//   713d0005 (rtk-rover >= 0.46.0): 16 B binary frame, decoded here and
//            emitted as headtrackerSampleReceived(azimuth, elevation, accel)
//   713d0002 (RWAHT): legacy ASCII text, emitted raw via
//            headtrackerDataReceived()
//   anything else (e.g. the RTK raw position on 713d0004) is ignored - it
//            used to be mis-parsed as heading.

#ifndef DEVICEHANDLER_H
#define DEVICEHANDLER_H

#include "bluetoothbaseclass.h"

#include <QLowEnergyController>
#include <QLowEnergyService>

class DeviceInfo;

class DeviceHandler : public BluetoothBaseClass
{
    Q_OBJECT

    Q_PROPERTY(bool alive READ alive NOTIFY aliveChanged)
    Q_PROPERTY(AddressType addressType READ addressType WRITE setAddressType)

public:
    enum class AddressType {
        PublicAddress,
        RandomAddress
    };
    Q_ENUM(AddressType)

    DeviceHandler(QObject *parent = nullptr);

    void setDevice(DeviceInfo *device);
    void setAddressType(AddressType type);
    AddressType addressType() const;

    bool alive() const;

signals:
    void aliveChanged();
    // Emitted for every ASCII notification from an RWAHT headtracker
    // (713d0002). The payload is the raw string
    // ("azimuth elevation linAccelZ").
    void headtrackerDataReceived(const QString &data);
    // Emitted for every decoded binary heading frame from an RTK
    // headtracker (713d0005, rtk-rover >= 0.46.0). Degrees / m/s^2,
    // calibration offsets not yet applied.
    void headtrackerSampleReceived(float azimuthDeg, float elevationDeg,
                                   float linAccelZ);

public slots:
    void disconnectService();

private:
    //QLowEnergyController
    void serviceDiscovered(const QBluetoothUuid &);
    void serviceScanDone();

    //QLowEnergyService
    void serviceStateChanged(QLowEnergyService::ServiceState s);
    void handleCharacteristicData(const QLowEnergyCharacteristic &c,
                                  const QByteArray &value);
    void confirmedDescriptorWrite(const QLowEnergyDescriptor &d,
                                  const QByteArray &value);

    QLowEnergyController *m_control = nullptr;
    QLowEnergyService *m_service = nullptr;
    // Every CCCD subscribed on the RWA service; drained one confirmed
    // unsubscribe at a time in confirmedDescriptorWrite on disconnect.
    QList<QLowEnergyDescriptor> m_notificationDescs;
    DeviceInfo *m_currentDevice = nullptr;

    bool m_foundHeadtrackerService = false;
    QLowEnergyController::RemoteAddressType m_addressType = QLowEnergyController::PublicAddress;
};

#endif // DEVICEHANDLER_H
