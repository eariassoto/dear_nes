// Copyright (c) 2020-2021 Emmanuel Arias
#include <soloud.h>
#include <soloud_speech.h>
#include <soloud_thread.h>
#include <iostream>
#include <cassert>

#include "include/simple_wave.h"

int main(int argc, char* argv[]) {
    // Define a couple of variables
    SoLoud::Soloud soloud;  // SoLoud engine core
    SoLoud::Speech speech;  // A sound source (speech, in this case)

    SimpleWave simpleWave;

    simpleWave.setSamplerate(0.9f);
    simpleWave.setFreq(0.5f);
    simpleWave.setWaveform(100);
    simpleWave.setVolume(1);

    auto instance = simpleWave.createInstance();

    // Configure sound source
    speech.setText("Hello world");

    // initialize SoLoud.
    soloud.init();
    soloud.setGlobalVolume(1.f);

    // Play the sound source (we could do this several times if we wanted)
    int handle = soloud.play(simpleWave);
    
    // Wait until sounds have finished
    while (soloud.getActiveVoiceCount() > 0) {
        // Still going, sleep for a bit
        assert(soloud.isValidVoiceHandle(handle));
        SoLoud::Thread::sleep(100);
    }

    // Clean up SoLoud
    soloud.deinit();

    // All done.
    return 0;
}

/*
#include <chrono>

#include "dear_nes_lib/cartridge_loader.h"
#include "dear_nes_lib/nes.h"
#include "helpers/RootDir.h"
#include "include/dearnes_window_manager.h"
#include "include/global_nes.h"
#include "include/status_widget.h"

using Nes = dearnes::Nes;
using Cartridge = dearnes::Cartridge;

int main(int argc, char* argv[]) {
    DearNESWindowManager dearNESWindowManager;
    bool success = dearNESWindowManager.CreateWindow();
    if (!success) {
        return 1;
    }
    dearNESWindowManager.RegisterWidgets();

    Nes* nesEmulator = g_GetGlobalNes();

    if (argc > 1) {
        dearnes::CartridgeLoader cartridgeLoader;
        std::string cartridgePath = ROOT_DIR "res/roms/" + std::string(argv[1]);
        auto ret = cartridgeLoader.LoadNewCartridge(cartridgePath);
        if (auto pval = std::get_if<Cartridge*>(&ret)) {
            nesEmulator->InsertCatridge(*pval);
        }
    }
    nesEmulator->Reset();

    auto nesStatusWindow = dearNESWindowManager.m_NesStatusWindow;

    constexpr float frameTime = 1.f / 60;
    typedef std::chrono::high_resolution_clock clock;
    typedef std::chrono::duration<float> duration;

    float residualTime = 0.f;
    auto previousTime = std::chrono::steady_clock::now();
    while (!dearNESWindowManager.ShouldClose()) {
        auto currentTime = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(
                              std::chrono::steady_clock::now() - previousTime)
                              .count();
        previousTime = currentTime;

        dearNESWindowManager.HandleEvents();

        if (nesStatusWindow->IsNesPoweredUp() &&
            nesEmulator->IsCartridgeLoaded()) {
            if (residualTime > 0) {
                residualTime -= deltaTime;
            } else {
                residualTime += frameTime - deltaTime;

                nesEmulator->DoFrame();
            }
        }

        dearNESWindowManager.Update(deltaTime);
        dearNESWindowManager.Render();
    }

    dearNESWindowManager.DestroyWindow();

    return 0;
}
*/
