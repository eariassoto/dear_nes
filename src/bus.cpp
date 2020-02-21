// Copyright (c) 2020 Emmanuel Arias
#include <iostream>

#include "include/bus.h"
#include "include/cartridge.h"
#include "include/logger.h"

namespace cpuemulator {
Bus::Bus() {
    m_Cpu->ConnectBus(this);
    memset(m_cpuRam, 0, 0x800);
}

Bus::~Bus() { delete[] m_cpuRam; }

std::shared_ptr<Cpu> Bus::GetCpuReference() { return m_Cpu; }

std::shared_ptr<Ppu> Bus::GetPpuReference() { return m_Ppu; }

uint64_t Bus::GetSystemClockCounter() const { return m_SystemClockCounter; }

void Bus::CpuWrite(uint16_t address, uint8_t data) {
    if (m_Cartridge->CpuWrite(address, data)) {
    } else if (address >= 0x0000 && address <= 0x1FFF) {
        m_cpuRam[GetRealRamAddress(address)] = data;
    } else if (address >= 0x2000 && address <= 0x3FFF) {
        m_Ppu->CpuWrite(GetRealPpuAddress(address), data);
    } else if (address == 0x4014) {
        m_DMAPage = data;
        m_DMAAddress = 0x00;
        m_DMATransfer = true;
    } else if (address >= 0x4016 && address <= 0x4017) {
        m_ControllerState[address & 0x0001] = m_Controllers[address & 0x0001];
    }
}

uint8_t Bus::CpuRead(uint16_t address, bool isReadOnly) {
    uint8_t data = 0x00;
    if (m_Cartridge->CpuRead(address, data)) {
    } else if (address >= 0x0000 && address <= 0x1FFF) {
        data = m_cpuRam[GetRealRamAddress(address)];
    } else if (address >= 0x2000 && address <= 0x3FFF) {
        data = m_Ppu->CpuRead(GetRealPpuAddress(address), isReadOnly);
    } else if (address >= 0x4016 && address <= 0x4017) {
        data = (m_ControllerState[address & 0x0001] & 0x80) > 0;
        m_ControllerState[address & 0x0001] <<= 1;
    }

    return data;
}

void Bus::InsertCatridge(const std::shared_ptr<Cartridge>& cartridge) {
    Logger::Get().Log("BUS", "Inserting cartridge");
    m_Cartridge = cartridge;

    m_Ppu->ConnectCatridge(cartridge);
}

void Bus::Reset() {
    m_Cpu->Reset();
    m_SystemClockCounter = 0;
}

void Bus::Clock() {
    m_Ppu->Clock();
    if (m_SystemClockCounter % 3 == 0) {
        if (m_DMATransfer) {
            if (m_DMADummy) {
                if (m_SystemClockCounter % 2 == 1) {
                    m_DMADummy = false;
                }
            } else {
                if (m_SystemClockCounter % 2 == 0) {
                    m_DMAData = CpuRead(m_DMAPage << 8 | m_DMAAddress);
                } else {
                    // todo offer api function
                    m_Ppu->m_OAMPtr[m_DMAAddress] = m_DMAData;
                    ++m_DMAAddress;

                    if (m_DMAAddress == 0) {
                        m_DMATransfer = false;
                        m_DMADummy = true;
                    }
                }
            }
        } else {
            m_Cpu->Clock();
        }
    }

    if (m_Ppu->m_DoNMI) {
        m_Ppu->m_DoNMI = false;
        m_Cpu->NonMaskableInterrupt();
    }

    ++m_SystemClockCounter;
}

}  // namespace cpuemulator
