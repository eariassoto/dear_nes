// Copyright (c) 2020 Emmanuel Arias
#include "dear_nes_lib/cartridge.h"
#include "dear_nes_lib/mapper.h"

namespace dearnes {

Cartridge::Cartridge(CartridgeHeader&& header, IMapper* mapper,
                     std::vector<uint8_t>&& programMemory,
                     std::vector<uint8_t>&& characterMemory)
    : m_CartridgeHeader{header},
      m_Mapper{mapper},
      m_ProgramMemory{programMemory},
      m_CharacterMemory{characterMemory} {}

Cartridge::~Cartridge() { delete m_Mapper; }

CartridgeHeader::MIRRORING_MODE Cartridge::GetMirroringMode() const {
    return m_CartridgeHeader.GetMirroringMode();
}

bool Cartridge::CpuRead(uint16_t address, uint8_t& data) {
    uint32_t mappedAddr = 0;
    if (m_Mapper->CpuMapRead(address, mappedAddr)) {
        data = m_ProgramMemory[mappedAddr];
        return true;
    }
    return false;
}

bool Cartridge::CpuWrite(uint16_t address, uint8_t data) {
    uint32_t mappedAddr = 0;
    if (m_Mapper->CpuMapWrite(address, mappedAddr)) {
        m_ProgramMemory[mappedAddr] = data;
        return true;
    }
    return false;
}

bool Cartridge::PpuRead(uint16_t address, uint8_t& data) {
    uint32_t mappedAddr = 0;
    if (m_Mapper->PpuMapRead(address, mappedAddr)) {
        data = m_CharacterMemory[mappedAddr];
        return true;
    }
    return false;
}

bool Cartridge::PpuWrite(uint16_t address, uint8_t data) {
    uint32_t mappedAddr = 0;
    if (m_Mapper->PpuMapWrite(address, mappedAddr)) {
        m_CharacterMemory[mappedAddr] = data;
        return true;
    }
    return false;
}

}  // namespace dearnes