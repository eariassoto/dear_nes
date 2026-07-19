// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include "dear_nes_lib/cartridge_header.h"

namespace dearnes {

// Forward declarations
class IMapper;
class CartridgeLoader;

/// <summary>
/// Holds the information and data from a valid NES game cartridge. This
/// class only supports cartridges that comply to the iNES file format.
/// It also supports only cartrigdes with mappers that the emulator has
/// implemented. For more information about this format, refer to
/// https://wiki.nesdev.com/w/index.php/INES.
/// </summary>
class Cartridge {
   public:

    Cartridge(CartridgeHeader&& header, IMapper* mapper,
              std::vector<uint8_t>&& programMemory,
              std::vector<uint8_t>&& characterMemory);

    ~Cartridge();

    CartridgeHeader::MIRRORING_MODE GetMirroringMode() const;

    /// <summary>
    /// Attempt to read from the CPU to the cartridge memory. If the mapper
    /// determines that the address is not in its domain, returns false and do
    /// nothing. Otherwise, read the data into the reference data param.
    /// </summary>
    /// <param name="address"></param>
    /// <param name="data"></param>
    /// <returns></returns>
    bool CpuRead(uint16_t address, uint8_t& data);

    /// <summary>
    /// Attempt to write from the CPU to the cartridge memory. If the mapper
    /// determines that the address is not in its domain, returns false and do
    /// nothing. Otherwise, write the data in memory.
    /// </summary>
    /// <param name="address"></param>
    /// <param name="data"></param>
    /// <returns></returns>
    bool CpuWrite(uint16_t address, uint8_t data);

    /// <summary>
    /// Attempt to read from the PPU to the cartridge memory. If the mapper
    /// determines that the address is not in its domain, returns false and do
    /// nothing. Otherwise, read the data into the reference data param.
    /// </summary>
    /// <param name="address"></param>
    /// <param name="data"></param>
    /// <returns></returns>
    bool PpuRead(uint16_t address, uint8_t& data);

    /// <summary>
    /// Attempt to write from the PPU to the cartridge memory. If the mapper
    /// determines that the address is not in its domain, returns false and do
    /// nothing. Otherwise, write the data in memory.
    /// </summary>
    /// <param name="address"></param>
    /// <param name="data"></param>
    /// <returns></returns>
    bool PpuWrite(uint16_t address, uint8_t data);

   private:
    CartridgeHeader m_CartridgeHeader;

    IMapper* m_Mapper = nullptr;

    std::vector<uint8_t> m_ProgramMemory;
    std::vector<uint8_t> m_CharacterMemory;
};
}  // namespace dearnes
