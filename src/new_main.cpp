// Copyright (c) 2026 Emmanuel Arias
#include <fmt/base.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

struct iNesHeader {
    // Must be $4E $45 $53 $1A ("NES\x1A")
    char m_MagicConstant[4];
    // Size of PRG ROM in 16 KB units
    uint8_t m_PrgRomChunks;
    // Size of CHR ROM in 8 KB units
    uint8_t m_ChrRomChunks;
    uint8_t m_Mapper1;
    uint8_t m_Mapper2;
    uint8_t m_PrgRamSize;
    uint8_t m_TvSystem1;
    uint8_t m_TvSystem2;
    char m_UnusedPadding[5];
};

struct CpuState {
    uint8_t reg_a_;
    uint8_t reg_x_;
    uint8_t reg_y_;
    uint8_t sp_;
    uint8_t status_;
    uint16_t pc_;
    char ram_[0xFFFF];
};

int main() {
    const std::string rom_path = "res/roms/nestest.nes";

    std::ifstream file(rom_path, std::ios::binary);
    if (!file.is_open()) {
        fmt::println("Failed to open ROM file: {}", rom_path);
        return 1;
    }

    iNesHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(iNesHeader));

    if (header.m_MagicConstant[0] != 'N' || header.m_MagicConstant[1] != 'E' ||
        header.m_MagicConstant[2] != 'S' || header.m_MagicConstant[3] != 0x1A) {
        fmt::println("Invalid iNES header magic bytes in {}", rom_path);
        return 1;
    }

    size_t prg_size = static_cast<size_t>(header.m_PrgRomChunks) * 16384;

    CpuState* cpu_state = new CpuState();
    
    file.read(&cpu_state->ram_[0x8000], prg_size);
    do {
        // TODO: Tick CPU
    } while (cpu_state->ram_[0x0002] != 0x00);
    
    delete cpu_state;
    return 0;
}
