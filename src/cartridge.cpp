// Copyright (c) 2020 Emmanuel Arias
#include "include/cartridge.h"
#include <fstream>
#include "include/logger.h"
#include "include/mapper_000.h"
#include "helpers/RootDir.h"

namespace cpuemulator {
Cartridge::Cartridge(const std::string& fileName) {
    // iNES Format
    struct Header {
        char name[4];
        uint8_t prgRomChunks;
        uint8_t chrRomChunks;
        uint8_t mapper1;
        uint8_t mapper2;
        uint8_t prg_ram_size;
        uint8_t tv_system1;
        uint8_t tv_system2;
        char unused[5];
    } header;

    std::ifstream ifs;
    ifs.open(ROOT_DIR "res/roms/" + fileName, std::ifstream::binary);
    if (!ifs.is_open()) {
        Logger::Get().Log("CART", "File {} not found", fileName);
        return;
    }
    ifs.read((char*)&header, sizeof(header));

    if (header.mapper1 & 0x04) {
        ifs.seekg(512, std::ios_base::cur);
    }
    m_MapperId = ((header.mapper2 >> 4) << 4) | (header.mapper1 >> 4);

    mirror = (header.mapper1 & 0x01) ? VERTICAL : HORIZONTAL;

    Logger::Get().Log("CART", "Mapper ID: {}", m_MapperId);

    // TODO: support file types

    // file type 1
    m_NumPrgBanks = header.prgRomChunks;
    m_ProgramMemory.resize(m_NumPrgBanks * 16384);
    ifs.read((char*)m_ProgramMemory.data(), m_ProgramMemory.size());
    Logger::Get().Log("CART", "Number of program banks: {}", m_NumPrgBanks);

    m_NumChrBanks = header.chrRomChunks;
    m_CharacterMemory.resize(m_NumChrBanks * 8192);
    ifs.read((char*)m_CharacterMemory.data(), m_CharacterMemory.size());
    Logger::Get().Log("CART", "Number of character banks: {}", m_NumChrBanks);

    ifs.close();

    switch (m_MapperId) {
        case 0:
            m_Mapper =
                std::make_unique<Mapper_000>(m_NumPrgBanks, m_NumChrBanks);
            break;
        default:
            Logger::Get().Log("CART", "Mapper ID {} not supported yet", m_MapperId);
            break;
    }
    if (m_Mapper != nullptr)
    {
        m_IsLoaded = true;
        Logger::Get().Log("CART", "Cartridge {} intialized successfully", fileName);
    }
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

}  // namespace cpuemulator