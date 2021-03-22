// Copyright (c) 2021 Emmanuel Arias
#include "include/simple_wave.h"

#include <soloud_misc.h>
#include <iostream>

SimpleWaveInstance::SimpleWaveInstance(SimpleWave *aParent) {
    mParent = aParent;
    mOffset = 0;
    mFreq = aParent->mFreq;
    mT = 0;
}

unsigned int SimpleWaveInstance::getAudio(float *aBuffer,
                                          unsigned int aSamplesToRead,
                                          unsigned int aBufferSize) {
    unsigned int i;
    int waveform = mParent->mWaveform;
    float d = 1.0f / mSamplerate;
    for (i = 0; i < aSamplesToRead; i++) {
        aBuffer[i] = -1;
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
    setSamplerate(44100);
    mWaveform = SoLoud::Soloud::WAVE_SQUARE;
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
