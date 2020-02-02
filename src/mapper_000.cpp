#include "include/mapper_000.h"

namespace cpuemulator {
Mapper_000::Mapper_000(uint8_t prgBanks, uint8_t chrBanks)
    : Mapper{prgBanks, chrBanks} {}

bool Mapper_000::CpuMapRead(uint16_t addr, uint32_t& mappedAddr) {
    if (addr >= 0x8000 && addr <= 0xFFFF) {
        mappedAddr = addr & (m_PrgBanks > 1 ? 0x7FFF : 0x3FFF);
        return true;
    }
    return false;
}

bool Mapper_000::CpuMapWrite(uint16_t addr, uint32_t& mappedAddr) {
    if (addr >= 0x8000 && addr <= 0xFFFF) {
        mappedAddr = addr & (m_PrgBanks > 1 ? 0x7FFF : 0x3FFF);
        return true;
    }
    return false;
}

bool Mapper_000::PpuMapRead(uint16_t addr, uint32_t& mappedAddr) {
    if (addr >= 0x0000 && addr <= 0x1FFF) {
        mappedAddr = addr;
        return true;
    }
    return false;
}

bool Mapper_000::PpuMapWrite(uint16_t addr, uint32_t& mappedAddr) {
    // no writing in ROM
    return false;
}

}  // namespace cpuemulator