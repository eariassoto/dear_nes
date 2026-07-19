// Copyright (c) 2020 Emmanuel Arias
#pragma once

#include <cstdint>

#include "dear_nes_lib/bus.h"
#include "dear_nes_lib/cpu.h"
#include "dear_nes_lib/dma.h"
#include "dear_nes_lib/ppu.h"

namespace dearnes {

// Forward declarations
class Cartridge;

class Nes {
   public:
    /// <summary>
    /// Constructs a new NES instance
    /// </summary>
    Nes();

    /// <summary>
    /// Default destructor
    /// </summary>
    ~Nes();

    /// <summary>
    /// Returns a global counter of ticks since creation
    /// </summary>
    /// <returns></returns>
    uint64_t GetSystemClockCounter() const;

    /// <summary>
    /// Insert a new virtual cartridge into the emulator. This will likely
    /// to be refactored into a better pattern
    /// </summary>
    /// <param name="cartridge"></param>
    void InsertCatridge(Cartridge* cartridge);

    /// <summary>
    /// Simulate the IRL reset routine
    /// </summary>
    void Reset();

    /// <summary>
    /// Run a single NES tick. The PPU will always tick. Every three ticks,
    /// the CPU will tick, unless there is a DMA transfer in progress.
    /// </summary>
    void Clock();

    /// <summary>
    /// Tick the emulator until a frame is completed.
    /// </summary>
    void DoFrame();

    /// <summary>
    /// Verify that a cartridge has been fully loaded. This will be rework into
    /// a better pattern.
    /// </summary>
    /// <returns></returns>
    bool IsCartridgeLoaded() const;

    /// <summary>
    /// Get the register for a particular virtual controller.
    /// The virtual controller #1 is identified by index 0
    /// and controller #2 by 1.
    /// </summary>
    /// <param name="controllerIdx"></param>
    /// <returns></returns>
    uint8_t GetControllerState(size_t controllerIdx) const;

    /// <summary>
    /// Clear all inputs from a controller. This will be refactored in the
    /// future.
    /// </summary>
    /// <param name="controllerIdx"></param>
    void ClearControllerState(size_t controllerIdx);

    /// <summary>
    /// Write to the register of the controller. It is not the ideal
    /// function because it will OR the parameter to the current register.
    /// This will be refactored in the future.
    /// The order of the buttons follows:
    /// 0 - A
    /// 1 - B
    /// 2 - Select
    /// 3 - Start
    /// 4 - Up
    /// 5 - Down
    /// 6 - Left
    /// 7 - Right
    ///
    /// </summary>
    /// <param name="controllerIdx">Index to identify the controller (0 for
    /// Controller #1, 1 for #2)</param> <param name="data">Mask to apply to the
    /// controller state</param>
    void WriteControllerState(size_t controllerIdx, uint8_t data);

    /// <summary>
    /// Returns a pointer to the PPU module
    /// </summary>
    /// <returns></returns>
    inline Ppu* GetPpu() { return &m_Ppu; }

    /// <summary>
    /// Returns a pointer to the CPU module
    /// </summary>
    /// <returns></returns>
    inline Cpu* GetCpu() { return &m_Cpu; }

   private:
    Bus m_Bus;
    Dma m_Dma;
    Ppu m_Ppu;
    Cpu m_Cpu;

    Cartridge* m_Cartridge = nullptr;

    bool m_IsCartridgeLoaded = false;

    uint32_t m_SystemClockCounter = 0;
};
}  // namespace dearnes