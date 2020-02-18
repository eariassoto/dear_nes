// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <memory>
#include <cstdint>
#include "include/cpu.h"
#include "include/ppu.h"

namespace cpuemulator {

class Cartridge;

class Cpu;

class Bus {
   public:
    Bus();
    ~Bus();

    inline const uint8_t* GetRamPointer() const { return m_cpuRam; }

    // todo do not expose
    Cpu m_Cpu;
    Ppu m_Ppu;
    uint32_t m_SystemClockCounter = 0;

   private:
    uint8_t* m_cpuRam = nullptr;

    std::shared_ptr<Cartridge> m_Cartridge = nullptr;

    inline uint16_t GetRealRamAddress(uint16_t address) const {
        return address & 0x07FF;
    }
    inline uint16_t GetRealPpuAddress(uint16_t address) const {
        return address & 0x0007;
    }

   public:
    void CpuWrite(uint16_t address, uint8_t data);
    uint8_t CpuRead(uint16_t address, bool isReadOnly = false);

    uint8_t m_Controllers[2] = {0};

    public : void
             InsertCatridge(const std::shared_ptr<Cartridge>& cartridge);
    void Reset();
    void Clock();

    uint8_t m_ControllerState[2] = {0};
};
}  // namespace cpuemulator