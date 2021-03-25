// Copyright (c) 2021 Emmanuel Arias
#include "include/dearnes_audio_source.h"

#include "dear_nes_lib/nes.h"

DearNESAudioSourceInstance::DearNESAudioSourceInstance(
    DearNESAudioSource *aParent)
    : m_Parent{aParent} {}

unsigned int DearNESAudioSourceInstance::getAudio(float *aBuffer,
                                                  unsigned int aSamplesToRead,
                                                  unsigned int aBufferSize) {
    dearnes::Nes *nesPtr = m_Parent->m_NesPtr;
    for (unsigned int i = 0; i < aSamplesToRead; i++) {
        aBuffer[i] = 0.f;
    }
    return aSamplesToRead;
}

bool DearNESAudioSourceInstance::hasEnded() {
    // This audio source never ends.
    return 0;
}

DearNESAudioSource::DearNESAudioSource() { mBaseSamplerate = 44100.f; }

DearNESAudioSource::~DearNESAudioSource() { stop(); }

SoLoud::AudioSourceInstance *DearNESAudioSource::createInstance() {
    return new DearNESAudioSourceInstance(this);
}
