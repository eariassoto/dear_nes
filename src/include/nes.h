// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <cstdint>
#include <memory>

#include "include/cpu.h"
#include "include/nes_widget.h"
#include "include/ppu.h"

namespace cpuemulator {

class Cartridge;

class Cpu;

class Nes {
   public:
    Nes();
    ~Nes();

    uint8_t m_Controllers[2] = {0};
    uint64_t GetSystemClockCounter() const;

    void CpuWrite(uint16_t address, uint8_t data);
    uint8_t CpuRead(uint16_t address, bool isReadOnly = false);

    void InsertCatridge(const std::shared_ptr<Cartridge>& cartridge);
    void Reset();
    void Clock();

    void RenderWidgets();
    void Update();
    void DoFrame();
    void Render();

   private:
    uint8_t* m_cpuRam = new uint8_t[0x800];

    std::shared_ptr<Cartridge> m_Cartridge = nullptr;

    std::shared_ptr<Cpu> m_Cpu = std::make_shared<Cpu>();

    std::shared_ptr<Ppu> m_Ppu = std::make_shared<Ppu>();

    uint32_t m_SystemClockCounter = 0;

    uint8_t m_DmaPage = 0x00;
    uint8_t m_DmaAddress = 0x00;
    uint8_t m_DmaData = 0x00;

    bool m_DmaTransfer = false;
    bool m_DmaWait = true;

    uint8_t m_ControllerState[2] = {0};

    NesWidget m_NesWidget;

    inline uint16_t GetRealRamAddress(uint16_t address) const {
        return address & 0x07FF;
    }
    inline uint16_t GetRealPpuAddress(uint16_t address) const {
        return address & 0x0007;
    }
};
}  // namespace cpuemulator