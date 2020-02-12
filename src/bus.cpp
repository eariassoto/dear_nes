// Copyright (c) 2020 Emmanuel Arias
#include <iostream>

#include "include/bus.h"
#include "include/cartridge.h"
#include "include/logger.h"

namespace cpuemulator {
Bus::Bus() : m_cpuRam{new uint8_t[0x800]} {
    m_Cpu.ConnectBus(this);
    memset(m_cpuRam, 0, 0x800);
}

Bus::~Bus() { delete[] m_cpuRam; }

void Bus::CpuWrite(uint16_t address, uint8_t data) {
    if (m_Cartridge->CpuWrite(address, data)) {
    } else if (address >= 0x0000 && address <= 0x1FFF) {
        m_cpuRam[GetRealRamAddress(address)] = data;
    } else if (address >= 0x2000 && address <= 0x3FFF) {
        m_Ppu.CpuWrite(GetRealPpuAddress(address), data);
    }
}

uint8_t Bus::CpuRead(uint16_t address, bool isReadOnly) {
    uint8_t data = 0x00;
    if (m_Cartridge->CpuRead(address, data)) {
    } else if (address >= 0x0000 && address <= 0x1FFF) {
        data = m_cpuRam[GetRealRamAddress(address)];
    } else if (address >= 0x2000 && address <= 0x3FFF) {
        data = m_Ppu.CpuRead(GetRealPpuAddress(address), isReadOnly);
    }

    return data;
}

void Bus::InsertCatridge(const std::shared_ptr<Cartridge>& cartridge) {
    Logger::Get().Log("BUS", "Inserting cartridge");
    m_Cartridge = cartridge;

    m_Ppu.ConnectCatridge(cartridge);
}

void Bus::Reset() {
    m_Cpu.Reset();
    m_SystemClockCounter = 0;
}

void Bus::Clock()
{
    m_Ppu.Clock();
    if (m_SystemClockCounter % 3 == 0)
    {
        m_Cpu.Clock();
    }
    ++m_SystemClockCounter;
}

}  // namespace cpuemulator
