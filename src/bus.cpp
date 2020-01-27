#include <iostream>

#include "include/bus.h"

namespace cpuemulator {
Bus::Bus() {
    for (auto& i : m_RAM) i = 0;

    m_CPU.ConnectBus(this);
}
void Bus::Write(uint16_t address, uint8_t data) {
    if (address >= 0x0000 && address <= 0xFFF) {
        m_RAM[address] = data;
    }
}

uint8_t Bus::Read(uint16_t address, bool /*isReadOnly*/) {
    if (address >= 0x0000 && address <= 0xFFF) {
        return m_RAM[address];
    }

    return uint8_t();
}
}  // namespace cpuemulator