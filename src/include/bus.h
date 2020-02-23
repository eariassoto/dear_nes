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

   public:
    std::shared_ptr<Cpu> GetCpuReference();
    std::shared_ptr<Ppu> GetPpuReference();
    uint64_t GetSystemClockCounter() const;

   private:
    uint8_t* m_cpuRam = new uint8_t[0x800];

    std::shared_ptr<Cartridge> m_Cartridge = nullptr;

    std::shared_ptr<Cpu> m_Cpu = std::make_shared<Cpu>();

    std::shared_ptr<Ppu> m_Ppu = std::make_shared<Ppu>();

    uint32_t m_SystemClockCounter = 0;

   private:
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

   public:
    void InsertCatridge(const std::shared_ptr<Cartridge>& cartridge);
    void Reset();
    void Clock();

    uint8_t m_ControllerState[2] = {0};

	private:
    uint8_t m_DmaPage = 0x00;
    uint8_t m_DmaAddress = 0x00;
    uint8_t m_DmaData = 0x00;

	bool m_DmaTransfer = false;
	bool m_DmaWait = true;

};
}  // namespace cpuemulator