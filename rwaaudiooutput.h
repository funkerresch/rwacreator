/*
 * This file is part of the Rwa Creator.
 * An open-source cross-platform Middleware for creating interactive Soundwalks
 *
 * Copyright (C) 2015 - 2022 Thomas Resch
 *
 * License: MIT
 *
 * rwaaudiooutput.h
 * by Thomas Resch
 *
 */

#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include <math.h>
#include "z_libpd.h"
#include "m_pd.h"
#include "util/z_print_util.h"
#include "portaudio.h"
#include "pa_mac_core.h"
#include <mutex>
#include <QObject>
#include "pawrapper.h"

class audioProcessor : public QObject, public paWrapper
{
  Q_OBJECT
public:
    std::mutex pdMutex;
    audioProcessor(int vectorSize);
    ~audioProcessor();
protected:
  virtual int processingCallback(const void *inputBuffer,
                                 void *outputBuffer,
                                 unsigned long framesPerBuffer,
                                 const PaStreamCallbackTimeInfo* timeInfo,
                                 PaStreamCallbackFlags statusFlags);
private:
    int pdTicks;
};

#endif // AUDIOOUTPUT_H
