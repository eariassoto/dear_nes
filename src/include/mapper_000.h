#pragma once
#include <cstdint>
#include <memory>
#include "include/mapper.h"

namespace cpuemulator {

class Mapper_000 : public Mapper {
   public:
    Mapper_000(uint8_t prgBanks, uint8_t chrBanks);

    bool CpuMapRead(uint16_t addr, uint32_t &mappedAddr) override;
    bool CpuMapWrite(uint16_t addr, uint32_t &mappedAddr) override;
    bool PpuMapRead(uint16_t addr, uint32_t &mappedAddr) override;
    bool PpuMapWrite(uint16_t addr, uint32_t &mappedAddr) override;
};
}  // namespace cpuemulator
