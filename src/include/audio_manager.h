// Copyright (c) 2021 Emmanuel Arias
#pragma once
#include <soloud.h>
#include "include/dearnes_audio_source.h"

namespace dearnes {
class Nes;
};

class AudioManager {
   public:
    AudioManager() = default;
    ~AudioManager() = default;

    void Initialize(dearnes::Nes *nesPtr);
    void Shutdown();

   private:
    dearnes::Nes *m_NesPtr = nullptr;
    DearNESAudioSource m_DearNESAudioSource;
    SoLoud::Soloud m_Soloud;  // SoLoud engine core
};
