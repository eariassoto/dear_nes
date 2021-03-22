// Copyright (c) 2021 Emmanuel Arias
#pragma once
#include <soloud.h>

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

class SimpleWave;

class SimpleWaveInstance : public SoLoud::AudioSourceInstance {
    SimpleWave *mParent;
    float mFreq;
    int mOffset;
    float mT;

   public:
    SimpleWaveInstance(SimpleWave *aParent);
    virtual unsigned int getAudio(float *aBuffer, unsigned int aSamplesToRead,
                                  unsigned int aBufferSize);
    virtual bool hasEnded();
};

class SimpleWave : public SoLoud::AudioSource {
   public:
    ADSR mADSR;
    float mFreq;
    int mWaveform;
    SimpleWave();
    virtual ~SimpleWave();
    void setSamplerate(float aSamplerate);
    void setWaveform(int aWaveform);
    void setFreq(float aFreq);
    virtual SoLoud::AudioSourceInstance *createInstance();
};
