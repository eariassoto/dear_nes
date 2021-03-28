// Copyright (c) 2021 Emmanuel Arias
#include "include/dearnes_audio_source.h"

#include <soloud_misc.h>

#include "dear_nes_lib/nes.h"

DearNESAudioSourceInstance::DearNESAudioSourceInstance(
    DearNESAudioSource *aParent)
    : m_Parent{aParent}, mOffset{0}, mFreq{aParent->mFreq}, mT{0} {}

unsigned int DearNESAudioSourceInstance::getAudio(float *aBuffer,
                                                  unsigned int aSamplesToRead,
                                                  unsigned int aBufferSize) {
    int waveform = m_Parent->mWaveform;
    float d = 1.0f / mSamplerate;
    for (unsigned int i = 0; i < aSamplesToRead; i++) {
        aBuffer[i] = SoLoud::Misc::generateWaveform(
                         waveform, fmod(mFreq * (float)mOffset, 1.0f)) *
                     m_Parent->mADSR.val(mT, 10000000000000.0f);
        mOffset++;
        mT += d;
    }
    return aSamplesToRead;
}

bool DearNESAudioSourceInstance::hasEnded() {
    // This audio source never ends.
    return 0;
}

DearNESAudioSource::DearNESAudioSource()
    : mWaveform{SoLoud::Soloud::WAVE_SIN},
      mFreq{(float)(440.f / mBaseSamplerate)} {}

DearNESAudioSource::~DearNESAudioSource() { stop(); }

SoLoud::AudioSourceInstance *DearNESAudioSource::createInstance() {
    return new DearNESAudioSourceInstance(this);
}
