// Copyright (c) 2021 Emmanuel Arias
#pragma once
#include <soloud.h>

namespace dearnes {
class Nes;
};
class DearNESAudioSource;

class ADSR {
   public:
    float mA, mD, mS, mR;

    ADSR() {
        mA = 0.0f;
        mD = 0.0f;
        mS = 1.0f;
        mR = 0.0f;
    }

    ADSR(float aA, float aD, float aS, float aR) {
        mA = aA;
        mD = aD;
        mS = aS;
        mR = aR;
    }

    float val(float aT, float aRelTime) {
        if (aT < mA) {
            return aT / mA;
        }
        aT -= mA;
        if (aT < mD) {
            return 1.0f - ((aT / mD)) * (1.0f - mS);
        }
        aT -= mD;
        if (aT < aRelTime) return mS;
        aT -= aRelTime;
        if (aT >= mR) {
            return 0.0f;
        }
        return (1.0f - aT / mR) * mS;
    }
};

class DearNESAudioSourceInstance : public SoLoud::AudioSourceInstance {
   public:
    DearNESAudioSourceInstance(DearNESAudioSource *aParent);
    virtual unsigned int getAudio(float *aBuffer, unsigned int aSamplesToRead,
                                  unsigned int aBufferSize);
    virtual bool hasEnded();

   private:
    DearNESAudioSource *m_Parent = nullptr;
    float mFreq;
    int mOffset;
    float mT;
};

class DearNESAudioSource : public SoLoud::AudioSource {
   public:
    DearNESAudioSource();
    virtual ~DearNESAudioSource();

    virtual SoLoud::AudioSourceInstance *createInstance();

    dearnes::Nes *m_NesPtr = nullptr;

    ADSR mADSR;
    float mFreq;
    int mWaveform;
};
