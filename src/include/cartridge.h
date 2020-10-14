// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace cpuemulator {

class Mapper;

class Cartridge {
   public:
    explicit Cartridge(const std::string& fileName);
    explicit Cartridge(const std::wstring& fileName);

    enum class MIRROR {
        HORIZONTAL,
        VERTICAL,
        ONESCREEN_LO,
        ONESCREEN_HI
    } mirror = MIRROR::HORIZONTAL;

   private:
    bool m_IsLoaded = false;
    uint8_t m_MapperId = 0x00;
    uint8_t m_NumPrgBanks = 0x00;
    uint8_t m_NumChrBanks = 0x00;
    std::vector<uint8_t> m_ProgramMemory;
    std::vector<uint8_t> m_CharacterMemory;

    std::shared_ptr<Mapper> m_Mapper;

    void ConstructFromFile(std::ifstream& ifs);

   public:
    bool IsLoaded() const { return m_IsLoaded; }

    bool CpuRead(uint16_t address, uint8_t& data);
    bool CpuWrite(uint16_t address, uint8_t data);

    bool PpuRead(uint16_t address, uint8_t& data);
    bool PpuWrite(uint16_t address, uint8_t data);
};
}  // namespace cpuemulator
