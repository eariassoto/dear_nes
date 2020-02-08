// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <cstdint>
#include <memory>

namespace cpuemulator {

class Mapper {
   public:
    Mapper(uint8_t prgBanks, uint8_t chrBanks);

	virtual bool CpuMapRead(uint16_t addr, uint32_t &mappedAddr) = 0;
	virtual bool CpuMapWrite(uint16_t addr, uint32_t &mappedAddr) = 0;
	virtual bool PpuMapRead(uint16_t addr, uint32_t &mappedAddr) = 0;
	virtual bool PpuMapWrite(uint16_t addr, uint32_t &mappedAddr) = 0;

	protected:
    uint8_t m_PrgBanks = 0;
    uint8_t m_ChrBanks = 0;
};
}  // namespace cpuemulator
