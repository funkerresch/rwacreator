/***************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the QtBluetooth module of the Qt Toolkit.
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

#include "devicefinder.h"
#include "devicehandler.h"
#include "deviceinfo.h"

#include <QCoreApplication>
#include <QPermissions>

DeviceFinder::DeviceFinder(DeviceHandler *handler, QObject *parent):
    BluetoothBaseClass(parent),
    m_deviceHandler(handler)
{
    m_deviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    m_deviceDiscoveryAgent->setLowEnergyDiscoveryTimeout(15000);

    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &DeviceFinder::addDevice);
    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred, this,
            &DeviceFinder::scanError);

    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &DeviceFinder::scanFinished);
    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled, this, &DeviceFinder::scanFinished);
}

DeviceFinder::~DeviceFinder()
{
    qDeleteAll(m_devices);
    m_devices.clear();
}

void DeviceFinder::setTargetName(const QString &name)
{
    m_targetName = name;
}

void DeviceFinder::startSearch()
{
    // request ONLY the "Access" (central/scanner) communication mode.
    // default QBluetoothPermission also requests "Advertise" (peripheral) mode.
    // would require NSBluetoothPeripheralUsageDescription in Info.plist
    // restricting to "Access" depends only on NSBluetoothAlwaysUsageDescription
    QBluetoothPermission permission;
    permission.setCommunicationModes(QBluetoothPermission::Access);

    switch (qApp->checkPermission(permission)) {
    case Qt::PermissionStatus::Undetermined:
        // aks for bt permissions, then re-enter: the status is resolved to Granted/Denied
        // by the time the callback fires, so the cases below handle the outcome.
        setInfo(tr("Requesting Bluetooth permission..."));
        qApp->requestPermission(permission, this, [this](const QPermission &p) {
            qDebug() << "[BLE debug] Bluetooth permission result:" << p.status();
            if (p.status() == Qt::PermissionStatus::Granted) {
                startDiscovery();
            }
            else {
                qDebug() << "[BLE debug] Bluetooth permission denied.";
            }
        });
        return;
    case Qt::PermissionStatus::Denied:
        setError(tr("Bluetooth permission denied. Enable it in System Settings > Privacy & Security > Bluetooth."));
        return;
    case Qt::PermissionStatus::Granted:
        startDiscovery();
        return;
    }
}

void DeviceFinder::startDiscovery()
{
    clearMessages();
    m_deviceHandler->setDevice(nullptr);
    qDeleteAll(m_devices);
    m_devices.clear();

    emit devicesChanged();

    m_deviceDiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);

    // qDebug() << "[BLE debug] DeviceDiscoveryAgent after start(): isActive=" << m_deviceDiscoveryAgent->isActive();
    // qDebug() << "[BLE debug]    error=" << m_deviceDiscoveryAgent->error();
    // qDebug() << "[BLE debug]    (" << m_deviceDiscoveryAgent->errorString() << ")";
    // qDebug() << "[BLE debug]    supportedMethods=" << QBluetoothDeviceDiscoveryAgent::supportedDiscoveryMethods();

    emit scanningChanged();
    setInfo(tr("Scanning for devices..."));
}

void DeviceFinder::addDevice(const QBluetoothDeviceInfo &device)
{
    // If device is a BLE device, add it to the list
    if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        auto *info = new DeviceInfo(device);
        m_devices.append(info);
        setInfo(tr("BLE device found: %1").arg(device.name()));
        emit devicesChanged();

        // connect as soon as the configured headtracker shows up.
        if (!m_targetName.isEmpty() && device.name() == m_targetName) {
            setInfo(tr("Connecting to %1...").arg(m_targetName));
            connectToService(info->getAddress());
        }
    }
}

void DeviceFinder::scanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    if (error == QBluetoothDeviceDiscoveryAgent::PoweredOffError)
        setError(tr("The Bluetooth adaptor is powered off."));
    else if (error == QBluetoothDeviceDiscoveryAgent::InputOutputError)
        setError(tr("Writing or reading from the device resulted in an error."));
    else if (error == QBluetoothDeviceDiscoveryAgent::MissingPermissionsError)
        setError(tr("Bluetooth permission denied — enable it in "
                    "System Settings > Privacy & Security > Bluetooth."));
    else
        setError(tr("An unknown error has occurred."));
}

void DeviceFinder::scanFinished()
{
#ifdef SIMULATOR
    // Only for testing
    for (int i = 0; i < 4; i++)
        m_devices.append(new DeviceInfo(QBluetoothDeviceInfo()));
#endif

    if (m_devices.isEmpty())
        setError(tr("No Low Energy devices found."));
    else
        setInfo(tr("Scanning done."));

    emit scanningChanged();
    emit devicesChanged();
}

void DeviceFinder::connectToService(const QString &address)
{
    m_deviceDiscoveryAgent->stop();

    DeviceInfo *currentDevice = nullptr;
    for (QObject *entry : std::as_const(m_devices)) {
        auto device = qobject_cast<DeviceInfo *>(entry);
        if (device && device->getAddress() == address ) {
            currentDevice = device;
            break;
        }
    }

    if (currentDevice)
        m_deviceHandler->setDevice(currentDevice);

    clearMessages();
}

bool DeviceFinder::scanning() const
{
    return m_deviceDiscoveryAgent->isActive();
}

QVariant DeviceFinder::devices()
{
    return QVariant::fromValue(m_devices);
}
