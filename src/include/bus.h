#pragma once
#include <array>
#include <cinttypes>

#include "include/cpu.h"
namespace cpuemulator {
class Bus {
   public:
    Bus();

   private:
    Cpu m_CPU;

    // fake ram
    std::array<uint8_t, 64 * 1024> m_RAM;

   public:
    void Write(uint16_t address, uint8_t data);
    uint8_t Read(uint16_t address, bool isReadOnly = false);
};
}  // namespace cpuemulator