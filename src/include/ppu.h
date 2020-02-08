// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <cstdint>
#include <memory>

namespace cpuemulator {

class Cartridge;

class Ppu {
   private:
    uint8_t m_TableName[2][1024];
    uint8_t m_TablePalette[32];

   public:
    uint8_t CpuRead(uint16_t address, bool readOnly = false);
    void CpuWrite(uint16_t address, uint8_t data);

    uint8_t PpuRead(uint16_t address, bool readOnly = false);
    void PpuWrite(uint16_t address, uint8_t data);

   private:
    std::shared_ptr<Cartridge> m_Cartridge;

   public:
    void ConnectCatridge(const std::shared_ptr<Cartridge>& cartridge);
    void Clock();
};
}  // namespace cpuemulator
