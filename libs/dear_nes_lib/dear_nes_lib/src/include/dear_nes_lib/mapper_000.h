// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <cstdint>
#include <memory>

#include "dear_nes_lib/mapper.h"

namespace dearnes {

/// <summary>
/// Implementation of the NROM mapper, identified by the iNES format as mapper 000.
/// There is no banking switching in this format. For more info refer to:
/// https://wiki.nesdev.com/w/index.php/NROM
/// </summary>
class Mapper_000 : public IMapper {
   public:

    /// <summary>
    /// Class constructor. The number of banks come from the cartridge's header.
    /// </summary>
    /// <param name="prgBanks"></param>
    /// <param name="chrBanks"></param>
    Mapper_000(uint8_t prgBanks, uint8_t chrBanks);

    bool CpuMapRead(uint16_t addr, uint32_t &mappedAddr) override;
    bool CpuMapWrite(uint16_t addr, uint32_t &mappedAddr) override;
    bool PpuMapRead(uint16_t addr, uint32_t &mappedAddr) override;
    bool PpuMapWrite(uint16_t addr, uint32_t &mappedAddr) override;
};
}  // namespace dearnes
