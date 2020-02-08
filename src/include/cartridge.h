#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace cpuemulator {

class Mapper;

class Cartridge {
   public:
    Cartridge(const std::string& fileName);

   private:
    bool m_IsLoaded = false;
    uint8_t m_MapperId = 0x00;
    uint8_t m_NumPrgBanks = 0x00;
    uint8_t m_NumChrBanks = 0x00;
    std::vector<uint8_t> m_ProgramMemory;
    std::vector<uint8_t> m_CharacterMemory;

    std::shared_ptr<Mapper> m_Mapper;

   public:
    bool IsLoaded() const { return m_IsLoaded; }

    bool CpuRead(uint16_t address, uint8_t& data);
    bool CpuWrite(uint16_t address, uint8_t data);

    bool PpuRead(uint16_t address, uint8_t& data);
    bool PpuWrite(uint16_t address, uint8_t data);
};
}  // namespace cpuemulator
