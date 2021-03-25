// Copyright (c) 2020-2021 Emmanuel Arias
#include "include/audio_manager.h"

void AudioManager::Initialize(dearnes::Nes* nesPtr) {
    m_NesPtr = nesPtr;
    m_Soloud.init();

    m_DearNESAudioSource.m_NesPtr = nesPtr;

    m_Soloud.play(m_DearNESAudioSource);
}

void AudioManager::Shutdown() {
    m_Soloud.stopAll();
    m_Soloud.deinit();
}
