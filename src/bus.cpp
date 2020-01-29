#include <iostream>

#include "include/bus.h"

namespace cpuemulator {
Bus::Bus() {
    m_RAM = new uint8_t[0xFFFF];
    memset(m_RAM, 0, 0xFFFF);
}

Bus::~Bus() { delete[] m_RAM; }

void Bus::Write(uint16_t address, uint8_t data) {
    if (address >= 0x0000 && address <= 0xFFFF) {
        m_RAM[address] = data;
    }
}

uint8_t Bus::Read(uint16_t address, bool /*isReadOnly*/) {
    if (address >= 0x0000 && address <= 0xFFFF) {
        return m_RAM[address];
    }

    return uint8_t();
}
}  // namespace cpuemulator