// Copyright (c) 2020 Emmanuel Arias
#include "include/nes.h"

#include <iostream>

#include "include/cartridge.h"
#include "include/logger.h"
#include "include/ppu.h"

namespace cpuemulator {

Nes::Nes() : m_Virtual6502{new Virtual6502(this)}, m_Ppu{new Ppu()} {
    memset(m_CpuRam, 0, 0x800);
}

Nes::~Nes() {
    delete m_Virtual6502;
    delete[] m_CpuRam;
    delete m_Cartridge;
    delete m_Ppu;
}

uint64_t Nes::GetSystemClockCounter() const { return m_SystemClockCounter; }

void Nes::CpuWrite(uint16_t address, uint8_t data) {
    if (m_Cartridge->CpuWrite(address, data)) {
    } else if (address >= 0x0000 && address <= 0x1FFF) {
        m_CpuRam[GetRealRamAddress(address)] = data;
    } else if (address >= 0x2000 && address <= 0x3FFF) {
        m_Ppu->CpuWrite(GetRealPpuAddress(address), data);
    } else if (address == 0x4014) {
        m_DmaPage = data;
        m_DmaAddress = 0x00;
        m_DmaTransfer = true;
    } else if (address >= 0x4016 && address <= 0x4017) {
        m_ControllerState[address & 0x0001] = m_Controllers[address & 0x0001];
    }
}

uint8_t Nes::CpuRead(uint16_t address, bool isReadOnly) {
    uint8_t data = 0x00;
    if (m_Cartridge->CpuRead(address, data)) {
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

void Nes::InsertCatridge(Cartridge* cartridge) {
    Logger::Get().Log("BUS", "Inserting cartridge");
    m_Cartridge = cartridge;

    m_Ppu->ConnectCatridge(cartridge);
    m_IsCartridgeLoaded = true;
}

void Nes::Reset() {
    m_Virtual6502->Reset();
    m_SystemClockCounter = 0;
    m_DmaPage = 0x00;
    m_DmaAddress = 0x00;
    m_DmaData = 0x00;
    m_DmaWait = true;
    m_DmaTransfer = false;
}

void Nes::Clock() {
    m_Ppu->Clock();
    if (m_SystemClockCounter % 3 == 0) {
        if (m_DmaTransfer) {
            if (m_DmaWait) {
                if (m_SystemClockCounter % 2 == 1) {
                    m_DmaWait = false;
                }
            } else {
                if (m_SystemClockCounter % 2 == 0) {
                    m_DmaData = CpuRead(m_DmaPage << 8 | m_DmaAddress);
                } else {
                    // todo offer api function
                    m_Ppu->m_OAMPtr[m_DmaAddress] = m_DmaData;
                    ++m_DmaAddress;

                    if (m_DmaAddress == 0) {
                        m_DmaTransfer = false;
                        m_DmaWait = true;
                    }
                }
            }
        } else {
            m_Virtual6502->Clock();
        }
    }

    if (m_Ppu->m_DoNMI) {
        m_Ppu->m_DoNMI = false;
        m_Virtual6502->NonMaskableInterrupt();
    }

    ++m_SystemClockCounter;
}

void Nes::DoFrame() {
    do {
        Clock();
    } while (!m_Ppu->isFrameComplete);

    do {
        m_Virtual6502->Clock();
    } while (m_Virtual6502->IsCurrentInstructionComplete());

    m_Ppu->isFrameComplete = false;
}

bool Nes::IsCartridgeLoaded() const { return m_IsCartridgeLoaded; }

}  // namespace cpuemulator
