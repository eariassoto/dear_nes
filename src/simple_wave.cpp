// Copyright (c) 2021 Emmanuel Arias
#include "include/simple_wave.h"

#include <soloud_misc.h>
#include <iostream>
#include <cmath>
#include <time.h>

SimpleWaveInstance::SimpleWaveInstance(SimpleWave *aParent) {
    mParent = aParent;
    mOffset = 0;
    mFreq = aParent->mFreq;
    mT = 0;
}

unsigned int SimpleWaveInstance::getAudio(float *aBuffer,
                                          unsigned int aSamplesToRead,
                                          unsigned int aBufferSize) {
    int waveform = mParent->mWaveform;
    float d = 1.0f / mSamplerate;
    for (unsigned int i = 0; i < aSamplesToRead; i++) {
        aBuffer[i] = SoLoud::Misc::generateWaveform(
                         waveform, fmod(mFreq * (float)mOffset, 1.0f)) *
                     mParent->mADSR.val(mT, 10000000000000.0f);
        mOffset++;
        mT += d;
    }
    return aSamplesToRead;
}

bool SimpleWaveInstance::hasEnded() {
    // This audio source never ends.
    return 0;
}

SimpleWave::SimpleWave() {
    setSamplerate(44100.f);
    mWaveform = SoLoud::Soloud::WAVE_SIN;
}

SimpleWave::~SimpleWave() { stop(); }

void SimpleWave::setSamplerate(float aSamplerate) {
    mBaseSamplerate = aSamplerate;
    mFreq = (float)(440 / mBaseSamplerate);
}

void SimpleWave::setFreq(float aFreq) { mFreq = aFreq / mBaseSamplerate; }

void SimpleWave::setWaveform(int aWaveform) { mWaveform = aWaveform; }

SoLoud::AudioSourceInstance *SimpleWave::createInstance() {
    return new SimpleWaveInstance(this);
}
