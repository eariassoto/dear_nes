// Copyright (c) 2021 Emmanuel Arias
#pragma once
#include <soloud.h>

namespace dearnes {
class Nes;
};
class DearNESAudioSource;

class DearNESAudioSourceInstance : public SoLoud::AudioSourceInstance {
   public:
    DearNESAudioSourceInstance(DearNESAudioSource *aParent);
    virtual unsigned int getAudio(float *aBuffer, unsigned int aSamplesToRead,
                                  unsigned int aBufferSize);
    virtual bool hasEnded();

   private:
    DearNESAudioSource *m_Parent = nullptr;
};

class DearNESAudioSource : public SoLoud::AudioSource {
   public:
    DearNESAudioSource();
    virtual ~DearNESAudioSource();

    virtual SoLoud::AudioSourceInstance *createInstance();

    dearnes::Nes *m_NesPtr = nullptr;
};
