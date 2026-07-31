/*
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along with this program;
 * if not, see <http://www.gnu.org/licenses/>.
*/
#ifndef PAWRAPPER_H
#define PAWRAPPER_H

#include "portaudio.h"
#include <QString>
#include <iostream>

using namespace std;

class paWrapper
{
public:
    paWrapper();
    ~paWrapper();

    PaStream *stream;

    void initAudio();
    int rescanDevices();
    int startAudio();
    int stopAudio();
    int runOnce();
    int numberOfInputs = 1;
    int numberOfOutputs = 2;
    bool isOutputDevice(int deviceIndex);
    bool isInputDevice(int deviceIndex);

    inline int getApiCount()
    {
        return Pa_GetHostApiCount();
    }

    inline const char* getApiName(int apiIndex)
    {
        return Pa_GetHostApiInfo(apiIndex)->name;
    }

    inline int getHostApi()
    {
        return selectedHostApi;
    }

    inline int setHostApi()
    {
        return selectedHostApi;
    }

    inline int inputChannelCount()
    {
        return inputStreamParam.channelCount;
    }

    inline int outputChannelCount()
    {
        return outputStreamParam.channelCount;
    }

    //wrapper for devices
    inline int getDeviceCount()  { return Pa_GetDeviceCount();  }

    /** Host api and device name of a device, empty for an invalid index. */
    QString getDeviceName(int deviceIndex);

    /** Index of the device with the given getDeviceName(), -1 if it is gone. */
    int findDeviceByName(const QString &name, bool output);

    inline int getOutputDevice()
    {
        return outputStreamParam.device;
    }

    /** Picks a device explicitly, which from now on wins over the system default. */
    int setOutputDevice(int deviceIndex);
    int setInputDevice(int deviceIndex);

    /** Gives up the explicit choice and follows the system default again. */
    void useSystemDefaultOutputDevice();
    void useSystemDefaultInputDevice();

    inline bool hasExplicitOutputDevice() { return !explicitOutputDeviceName.isEmpty(); }
    inline bool hasExplicitInputDevice()  { return !explicitInputDeviceName.isEmpty(); }


    inline int getInputDevice()
    {
        return inputStreamParam.device;
    }

    //wrapper for sample rates
    inline int isSampleRateSupported(double sampleRate)
    {
        return Pa_IsFormatSupported(&inputStreamParam,&outputStreamParam,sampleRate);
    }

    inline double getSampleRate()
    {
        return sampleRate;
    }

    inline int setSampleRate(double sampleRate)
    {
        int err=isSampleRateSupported(sampleRate);
        if(err < 0)
            return err;

        else
        {
            this->sampleRate = sampleRate;
            return 0;
        }
    }

    inline double getDefaultSampleRate(int deviceIndex)
    {
        return Pa_GetDeviceInfo(deviceIndex)->defaultSampleRate;
    }

    //wrapper for frame length
    inline void setFrameLength(int frameLength)
    {
        this->frameLength = frameLength;
    }

    inline int getFrameLength()
    {
        return frameLength;
    }

    //misc wrapper methods
    const char* getErrorText(int errorId);

enum PawErrorCode
{
  pawErrorAudioIsRunning = -100
} pawErrorCode;

protected:
  /** Takes over the current default devices of the system, see initAudio()/rescanDevices(). */
  void applyDefaultDevices();

  /** Fills the stream parameters without touching the remembered explicit choice. */
  int applyOutputDevice(int deviceIndex);
  int applyInputDevice(int deviceIndex);

  /** Device picked by the user, empty means: follow the system default. */
  QString explicitOutputDeviceName;
  QString explicitInputDeviceName;

  PaTime suggestedLatency;

  PaHostApiIndex selectedHostApi;
  PaStreamParameters inputStreamParam;
  PaStreamParameters outputStreamParam;
  double sampleRate;
  int frameLength;
  bool isRunning; //flag signaling that the audio processing is running or not

  //data processing callback method
  virtual int processingCallback(const void *inputBuffer,
                                 void *outputBuffer,
                                 unsigned long framesPerBuffer,
                                 const PaStreamCallbackTimeInfo* timeInfo,
                                 PaStreamCallbackFlags statusFlags);

  //portaudio need a static member function as callback,
  //this calls the non-static processing callback method by passing the object via userData
  static int paStaticCallback(const void *inputBuffer,
                              void *outputBuffer,
                              unsigned long framesPerBuffer,
                              const PaStreamCallbackTimeInfo* timeInfo,
                              PaStreamCallbackFlags statusFlags,
                              void *userData)
  {
    return ((paWrapper*)userData)->processingCallback(inputBuffer, outputBuffer, framesPerBuffer, timeInfo, statusFlags);
  }

};


#endif // PAWRAPPER_H
