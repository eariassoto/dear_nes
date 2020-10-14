// Copyright (c) 2020 Emmanuel Arias
#pragma once

#include <cstdint>

#include "include/ppu.h"
#include "include/ui_config.h"
#include "include/virtual6502.h"
#include "include/imgui_cartridge_explorer.h"

namespace cpuemulator {

class Cartridge;

class Nes {
   public:
    explicit Nes(const UiConfig& uiConfig);
    ~Nes();

    uint8_t m_Controllers[2] = {0};
    uint64_t GetSystemClockCounter() const;

    void CpuWrite(uint16_t address, uint8_t data);
    uint8_t CpuRead(uint16_t address, bool isReadOnly = false);

    void InsertCatridge(Cartridge* cartridge);
    void Reset();
    void Clock();

    void RenderWidgets();
    void Update();
    void DoFrame();
    void Render();

    bool IsCartridgeLoaded() const;

   private:
    const UiConfig& m_UiConfig;

    uint8_t* m_CpuRam = new uint8_t[0x800];

    Cartridge* m_Cartridge = nullptr;

    bool m_IsCartridgeLoaded = false;

    Virtual6502<Nes>* m_Virtual6502 = nullptr;

    Ppu* m_Ppu = nullptr;

    uint32_t m_SystemClockCounter = 0;

    ImguiCartridgeExplorer m_CartridgeExplorer;

    uint8_t m_DmaPage = 0x00;
    uint8_t m_DmaAddress = 0x00;
    uint8_t m_DmaData = 0x00;

    bool m_DmaTransfer = false;
    bool m_DmaWait = true;

    uint8_t m_ControllerState[2] = {0};

    inline uint16_t GetRealRamAddress(uint16_t address) const {
        return address & 0x07FF;
    }
    inline uint16_t GetRealPpuAddress(uint16_t address) const {
        return address & 0x0007;
    }

    void RenderCpuWidget();
    void RenderControllerWidget();
};
}  // namespace cpuemulator