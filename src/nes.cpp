// Copyright (c) 2020 Emmanuel Arias
#include "dear_nes_lib/nes.h"

#include <cassert>
#include <iostream>

#include "dear_nes_lib/cartridge.h"
#include "dear_nes_lib/enums.h"

namespace dearnes {

Nes::Nes() {
    m_Bus.SetPpu(&m_Ppu);
    m_Bus.SetDma(&m_Dma);

    m_Dma.SetBus(&m_Bus);

    m_Cpu.SetBus(&m_Bus);
}

Nes::~Nes() {
    if (m_Cartridge != nullptr) {
        delete m_Cartridge;
    }
}

uint64_t Nes::GetSystemClockCounter() const { return m_SystemClockCounter; }

void Nes::InsertCatridge(Cartridge* cartridge) {
    m_Bus.SetCartridge(cartridge);
    m_Ppu.ConnectCatridge(cartridge);
    m_IsCartridgeLoaded = true;
    if (m_Cartridge != nullptr) {
        delete m_Cartridge;
    }
    m_Cartridge = cartridge;
    Reset();
}

void Nes::Reset() {
    if (!m_IsCartridgeLoaded) {
        return;
    }
    m_Cpu.Reset();
    m_SystemClockCounter = 0;
}

void Nes::Clock() {
    auto DoDMATransfer = [&]() {
        if (m_Dma.IsInWaitState()) {
            if (m_SystemClockCounter % 2 == 1) {
                m_Dma.StopWaiting();
            }
        } else {
            if (m_SystemClockCounter % 2 == 0) {
                m_Dma.ReadData();
            } else {
                auto [addr, data] = m_Dma.GetLastReadData();
                m_Ppu.m_OAMPtr[addr] = data;
            }
        }
    };
    m_Ppu.Clock();
    if (m_SystemClockCounter % 3 == 0) {
        if (m_Dma.IsTranferInProgress()) {
            DoDMATransfer();
        } else {
            m_Cpu.Clock();
        }
    }
    if (m_Ppu.NeedsToDoNMI()) {
        m_Cpu.NonMaskableInterrupt();
    }
    ++m_SystemClockCounter;
}

void Nes::DoFrame() {
    if (!m_IsCartridgeLoaded) {
        return;
    }
    do {
        Clock();
    } while (!m_Ppu.IsFrameCompleted());

    do {
        m_Cpu.Clock();
    } while (m_Cpu.IsCurrentInstructionComplete());

    m_Ppu.StartNewFrame();
}

bool Nes::IsCartridgeLoaded() const { return m_IsCartridgeLoaded; }

uint8_t Nes::GetControllerState(size_t controllerIdx) const {
    assert(controllerIdx < NUM_CONTROLLERS);
    return m_Bus.GetControllerState(controllerIdx);
}

void Nes::ClearControllerState(size_t controllerIdx) {
    assert(controllerIdx < NUM_CONTROLLERS);
    m_Bus.ClearControllerState(controllerIdx);
}

void Nes::WriteControllerState(size_t controllerIdx, uint8_t data) {
    assert(controllerIdx < NUM_CONTROLLERS);
    m_Bus.WriteControllerState(controllerIdx, data);
}

}  // namespace dearnes
