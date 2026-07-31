/*
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along with this program;
 * if not, see <http://www.gnu.org/licenses/>.
*/

#include "pawrapper.h"
#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <QDebug>

using namespace std;

//PaStream *stream; //now as class member?

// don't put (QT) logging in processingCallback(): it runs on the audio
// thread, where logging (allocating, locking) would cause dropouts.
int paWrapper::processingCallback(const void *inputBuffer,
                                  void *outputBuffer,
                                  unsigned long framesPerBuffer,
                                  const PaStreamCallbackTimeInfo* timeInfo,
                                  PaStreamCallbackFlags statusFlags)
{
    (void) inputBuffer; // Prevent unused variable warning.
    (void) outputBuffer; // Prevent unused variable warning.
    (void) framesPerBuffer; // Prevent unused variable warning.
    (void) timeInfo; // Prevent unused variable warning.
    (void) statusFlags; // Prevent unused variable warning.
    return 0;
}

paWrapper::paWrapper()
{
    isRunning=false; //currently nothing is running
    initAudio();
}

/**
  Selecting a device from the menu is an explicit choice by the user: it is remembered by name and
  wins over the system default on every following rescan, even if the device is temporarily
  unplugged in between. useSystemDefault*Device() gives that up again.
*/
int paWrapper::setInputDevice(int deviceIndex)
{
    int err = applyInputDevice(deviceIndex);
    if(err == 0)
        explicitInputDeviceName = getDeviceName(deviceIndex);

    return err;
}

int paWrapper::setOutputDevice(int deviceIndex)
{
    int err = applyOutputDevice(deviceIndex);
    if(err == 0)
        explicitOutputDeviceName = getDeviceName(deviceIndex);

    return err;
}

void paWrapper::useSystemDefaultInputDevice()
{
    explicitInputDeviceName.clear();
    applyInputDevice(Pa_GetDefaultInputDevice());
}

void paWrapper::useSystemDefaultOutputDevice()
{
    explicitOutputDeviceName.clear();
    applyOutputDevice(Pa_GetDefaultOutputDevice());
}

int paWrapper::applyInputDevice(int deviceIndex)
{
    if(deviceIndex < 0 || deviceIndex >= Pa_GetDeviceCount())
        return -1;

    const PaDeviceInfo* di = Pa_GetDeviceInfo(deviceIndex);
    inputStreamParam.device = deviceIndex;
    inputStreamParam.channelCount = di->maxInputChannels;
    inputStreamParam.sampleFormat = paFloat32;
    inputStreamParam.hostApiSpecificStreamInfo = NULL;

    return 0;
}

int paWrapper::applyOutputDevice(int deviceIndex)
{
    if(deviceIndex < 0 || deviceIndex >= Pa_GetDeviceCount())
        return -1;

    const PaDeviceInfo* di = Pa_GetDeviceInfo(deviceIndex);
    outputStreamParam.device = deviceIndex;
    outputStreamParam.channelCount = di->maxOutputChannels;
    outputStreamParam.sampleFormat = paFloat32;
    outputStreamParam.hostApiSpecificStreamInfo = NULL;

    return 0;
}

void paWrapper::initAudio()
{
    int err = Pa_Initialize();

    if( err != paNoError )
        qWarning() << "PortAudio error (Pa_Initialize):" << Pa_GetErrorText( err );

    sampleRate = 48000.0;
    applyDefaultDevices();
}

/**
  PortAudio enumerates the devices in Pa_Initialize() and never updates that list, so devices
  connected or removed while the app is running are invisible and the stored device indices go
  stale. Terminating and re-initializing is the only way to pick up the new device list.

  The scan then follows the system default; macOS already switches that to a headset when one is
  connected. A device the user picked from the menu overrides the default whenever
  it is present; it stays remembered by name while it is unplugged, so it is taken
  again once it comes back.

  Must not be called while the stream is open.
*/
int paWrapper::rescanDevices()
{
    if(isRunning)
        return pawErrorAudioIsRunning;

    int err = Pa_Terminate();
    if( err != paNoError )
    {
        qWarning() << "PortAudio error (Pa_Terminate):" << Pa_GetErrorText( err );
        return err;
    }

    err = Pa_Initialize();
    if( err != paNoError )
    {
        qWarning() << "PortAudio error (Pa_Initialize):" << Pa_GetErrorText( err );
        return err;
    }

    applyDefaultDevices();

    int deviceIndex = findDeviceByName(explicitOutputDeviceName, true);
    if(deviceIndex >= 0)
        applyOutputDevice(deviceIndex);

    deviceIndex = findDeviceByName(explicitInputDeviceName, false);
    if(deviceIndex >= 0)
        applyInputDevice(deviceIndex);

    return paNoError;
}

void paWrapper::applyDefaultDevices()
{
    const PaDeviceInfo* di;
    int inputChannels = 0;
    int outputChannels = 0;

    selectedHostApi = Pa_GetDefaultHostApi();

    int noOfHostApis = Pa_GetHostApiCount();
    int noOfAudioDevices = Pa_GetDeviceCount();

    // The full device list is logged as debug: a rescan runs on every simulation start and would
    // otherwise flood the log view. RwaSimulator::rescanAudioDevices() logs the outcome as info.
    qDebug() << noOfHostApis << "host apis and" << noOfAudioDevices << "audio devices available";

    PaDeviceIndex selectedOutputDevice = Pa_GetDefaultOutputDevice();
    PaDeviceIndex selectedInputDevice = Pa_GetDefaultInputDevice();

    for(int i=0; i < noOfAudioDevices; i++)
    {
        di = Pa_GetDeviceInfo(i);

        // qDebug() << "Device" << i << ":" << di->name;

        if(i == selectedInputDevice)
        {
            suggestedLatency = di->defaultLowInputLatency;
            inputChannels = di->maxInputChannels;

        }
        if(i == selectedOutputDevice)
        {
            suggestedLatency = di->defaultLowInputLatency;
            outputChannels = di->maxOutputChannels;
        }
    }

    outputStreamParam.channelCount = outputChannels;
    outputStreamParam.device = Pa_GetDefaultOutputDevice();
    outputStreamParam.sampleFormat = paFloat32;
    outputStreamParam.suggestedLatency = suggestedLatency;
    outputStreamParam.hostApiSpecificStreamInfo = NULL;

    inputStreamParam.channelCount = inputChannels;
    inputStreamParam.device = Pa_GetDefaultInputDevice();
    inputStreamParam.sampleFormat = paFloat32;
    inputStreamParam.suggestedLatency = suggestedLatency;
    inputStreamParam.hostApiSpecificStreamInfo = NULL;

    qDebug() << "Default output device:" << Pa_GetDefaultOutputDevice()
             << "- default input device:" << Pa_GetDefaultInputDevice();
}

paWrapper::~paWrapper()
{
    int err = Pa_Terminate();
    if( err != paNoError )
       qWarning() << "PortAudio error (Pa_Terminate):" << Pa_GetErrorText( err );
}

int paWrapper::startAudio(void)
{
    if(isRunning)
        return pawErrorAudioIsRunning;

    isRunning=true;

    int err = Pa_OpenStream(&stream,
                      &inputStreamParam,
                      &outputStreamParam,
                      sampleRate,
                      frameLength,
                      paNoFlag,
                      &paWrapper::paStaticCallback,
                      this);

    if(err != paNoError)
    {
        isRunning=false;
        return err;
    }

    err = Pa_StartStream( stream );
    if(err != paNoError)
        isRunning=false;

    return err;
}

int paWrapper::stopAudio(void)
{
    int err=-1;
    if(isRunning)
    {
        err = Pa_AbortStream( stream );
        isRunning=false;

        if( err != paNoError )
            return err;

        err = Pa_CloseStream( stream );
    }
    return err;
}

bool paWrapper::isInputDevice(int deviceIndex)
{
    const PaDeviceInfo* di;
    di = Pa_GetDeviceInfo(deviceIndex);
    if(di->maxInputChannels > 0)
        return true;
    else
        return false;
}

QString paWrapper::getDeviceName(int deviceIndex)
{
    if(deviceIndex < 0 || deviceIndex >= Pa_GetDeviceCount())
        return QString();

    const PaDeviceInfo* di = Pa_GetDeviceInfo(deviceIndex);
    if(!di)
        return QString();

    return QString("%1: %2").arg(Pa_GetHostApiInfo(di->hostApi)->name, di->name);
}

int paWrapper::findDeviceByName(const QString &name, bool output)
{
    if(name.isEmpty())
        return -1;

    for(int i = 0; i < Pa_GetDeviceCount(); i++)
    {
        if(output ? !isOutputDevice(i) : !isInputDevice(i))
            continue;

        if(getDeviceName(i) == name)
            return i;
    }

    return -1;
}

bool paWrapper::isOutputDevice(int deviceIndex)
{
    const PaDeviceInfo* di;
    di = Pa_GetDeviceInfo(deviceIndex);
    if(di->maxOutputChannels > 0)
        return true;
    else
        return false;
}

int paWrapper::runOnce(void)
{
    int err = startAudio();
    if( err != paNoError ) return err;

    // Sleep
    Pa_Sleep(4000);

    err = stopAudio();
    return err;
}

/**
  The wrapper for user messages. Replace text messages here for user convenience.
*/
const char* paWrapper::getErrorText(int errorId)
{
    switch(errorId)
    {
        case paInvalidChannelCount:
          return "No output channels available or device busy";

        case pawErrorAudioIsRunning:
          return "The audio processing is already running";

        default:
          return Pa_GetErrorText(errorId);
    }
}
