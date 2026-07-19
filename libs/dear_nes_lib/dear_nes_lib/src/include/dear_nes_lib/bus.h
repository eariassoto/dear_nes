// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <array>
#include <cinttypes>

#include "dear_nes_lib/enums.h"

namespace dearnes {

// Forward declarations
class Cartridge;
class Dma;
class Ppu;

/// <summary>
/// In charge of handling memory access for the CPU and the DMA module.
/// The CPU will be unaware from which place the memory comes from. A read
/// or write request from the CPU can be executed in the cartridge, the PPU,
/// the DMA (to start the transfer process), or to the controllers registers.
/// </summary>
class Bus {
   public:
    Bus();
    ~Bus() = default;

    /// <summary>
    ///  Write a byte into the memory. The function will give priority to
    ///  cartridge's mapper code. Otherwise, it will follow this mapping:
    /// 
    /// 0x0000 -> 0x1FFF: Write to CPU RAM
    /// 0x2000 -> 0x3FFF: Write to PPU internal memory
    /// 0x4014: Signal DMA module to start transfer
    /// 0x4016 -> 0x4017: Write to controller register
    /// 
    /// Writing to a invalid memory will do nothing.
    /// </summary>
    /// <param name="address">Address to write</param>
    /// <param name="data">Data to be written</param>
    void CpuWrite(uint16_t address, uint8_t data);

    /// <summary>
    /// Read a byte from memory. This function follows the same rules as CpuWrite
    /// </summary>
    /// <param name="address">Address to read</param>
    /// <param name="isReadOnly">Unused parameter. Some mappers might require
    /// information on whether they can write when reading.
    /// </param>
    /// <returns>Byte from memory</returns>
    uint8_t CpuRead(uint16_t address, bool isReadOnly = false);

    /// <summary>
    /// Load the current cartridge
    /// </summary>
    /// <param name="cartridge"></param>
    void SetCartridge(Cartridge* cartridge);

    /// <summary>
    /// Set the reference to the DMA module
    /// </summary>
    /// <param name="dma"></param>
    void SetDma(Dma* dma);

    /// <summary>
    /// Set the reference to the PPU module
    /// </summary>
    /// <param name="ppu"></param>
    void SetPpu(Ppu* ppu);

    // TODO: Provide better controller API
    
    /// <summary>
    /// Get the register for a particular virtual controller.
    /// The virtual controller #1 is identified by index 0 
    /// and controller #2 by 1.
    /// </summary>
    /// <param name="controllerIdx"></param>
    /// <returns></returns>
    uint8_t GetControllerState(size_t controllerIdx) const;
    
    /// <summary>
    /// Clear all inputs from a controller. This will be refactored in the future.
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
    /// <param name="controllerIdx">Index to identify the controller (0 for Controller #1, 1 for #2)</param>
    /// <param name="data">Mask to apply to the controller state</param>
    void WriteControllerState(size_t controllerIdx, uint8_t data);

   private:
    Cartridge* m_Cartridge = nullptr;
    Dma* m_Dma = nullptr;
    Ppu* m_Ppu = nullptr;

    uint8_t m_Controllers[NUM_CONTROLLERS] = {0};
    uint8_t m_ControllerState[NUM_CONTROLLERS] = {0};

    std::array<uint8_t, SIZE_CPU_RAM> m_CpuRam;

    inline uint16_t GetRealRamAddress(uint16_t address) const {
        return address & 0x07FF;
    }
    inline uint16_t GetRealPpuAddress(uint16_t address) const {
        return address & 0x0007;
    }
};

}  // namespace dearnes
