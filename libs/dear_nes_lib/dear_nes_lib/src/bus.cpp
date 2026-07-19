// Copyright (c) 2020 Emmanuel Arias
#include "dear_nes_lib/bus.h"

#include <cstring>
#include <cassert>

#include "dear_nes_lib/cartridge.h"
#include "dear_nes_lib/dma.h"
#include "dear_nes_lib/ppu.h"

namespace dearnes {

Bus::Bus() {
    m_CpuRam.fill(0x00);
}

void Bus::SetCartridge(Cartridge* cartridge) {
    assert(cartridge != nullptr);
    m_Cartridge = cartridge;
}

void Bus::SetDma(Dma* dma) {
    assert(dma != nullptr);
    m_Dma = dma;
}

void Bus::SetPpu(Ppu* ppu) {
    assert(ppu != nullptr);
    m_Ppu = ppu;
}

uint8_t Bus::GetControllerState(size_t controllerIdx) const {
    return m_Controllers[controllerIdx];
}

void Bus::ClearControllerState(size_t controllerIdx) {
    m_Controllers[controllerIdx] = 0x00;
}

void Bus::WriteControllerState(size_t controllerIdx, uint8_t data) {
    m_Controllers[controllerIdx] |= data;
}

void Bus::CpuWrite(uint16_t address, uint8_t data) {
    if (m_Cartridge && m_Cartridge->CpuWrite(address, data)) {
    } else if (address >= 0x0000 && address <= 0x1FFF) {
        m_CpuRam[GetRealRamAddress(address)] = data;
    } else if (address >= 0x2000 && address <= 0x3FFF) {
        m_Ppu->CpuWrite(GetRealPpuAddress(address), data);
    } else if (address == 0x4014) {
        m_Dma->StartTransfer(data);
    } else if (address >= 0x4016 && address <= 0x4017) {
        m_ControllerState[address & 0x0001] = m_Controllers[address & 0x0001];
    }
}

uint8_t Bus::CpuRead(uint16_t address, bool isReadOnly) {
    uint8_t data = 0x00;
    if (m_Cartridge && m_Cartridge->CpuRead(address, data)) {
    } else if (address >= 0x0000 && address <= 0x1FFF) {
        data = m_CpuRam[GetRealRamAddress(address)];
    } else if (address >= 0x2000 && address <= 0x3FFF) {
        data = m_Ppu->CpuRead(GetRealPpuAddress(address), isReadOnly);
    } else if (address >= 0x4016 && address <= 0x4017) {
        data = (m_ControllerState[address & 0x0001] & 0x80) > 0;
        m_ControllerState[address & 0x0001] <<= 1;
    }

    return data;
}

}  // namespace dearnes