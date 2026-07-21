// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <cstdint>
#include <fstream>

namespace dearnes {

/// <summary>
/// iNES format for virtual cartridge headers. This struct describes the
/// data located in the first 16 bytes of the cartridge. For more info refer
/// to: https://wiki.nesdev.com/w/index.php/INES
/// </summary>
class CartridgeHeader {
   public:
    CartridgeHeader() = delete;
    CartridgeHeader(std::ifstream& inputStream);

    /// <summary>
    /// Mirroring mode for the game. Refer to
    /// https://wiki.nesdev.com/w/index.php/Mirroring
    /// </summary>
    enum class MIRRORING_MODE {
        HORIZONTAL,
        VERTICAL,
        ONESCREEN_LO,
        ONESCREEN_HI
    };

    bool HasTrainerData() const;

    /// <summary>
    /// Returns the mirroring mode for the cartridge
    /// </summary>
    /// <returns></returns>
    MIRRORING_MODE GetMirroringMode() const;

    inline uint8_t GetMapperId() const {
        return ((m_iNesHeader.m_Mapper2 >> 4) << 4) | (m_iNesHeader.m_Mapper1 >> 4);
    };

    inline size_t GetProgramMemoryBanks() const {
        return m_iNesHeader.m_PrgRomChunks;
    }

    inline size_t GetCharacterMemoryBanks() const {
        return m_iNesHeader.m_ChrRomChunks;
    }

    inline size_t GetProgramMemorySize() const {
        return static_cast<size_t>(m_iNesHeader.m_PrgRomChunks) * 16384;
    }

    inline size_t GetCharacterMemorySize() const {
        return static_cast<size_t>(m_iNesHeader.m_ChrRomChunks) * 8192;
    }

   private:
    struct iNesHeader {
        /// <summary>
        /// Magic constant. Must be $4E $45 $53 $1A
        /// </summary>
        char m_MagicConstant[4];

        /// <summary>
        /// Size of PRG ROM in 16 KB units
        /// </summary>
        uint8_t m_PrgRomChunks;

        /// <summary>
        /// Size of CHR ROM in 8 KB units
        /// </summary>
        uint8_t m_ChrRomChunks;

        // clang-format off
        /// <summary>
        /// Mapper, mirroring, battery, trainer
        ///
        /// 76543210
        /// ||||||||
        /// |||||||+- Mirroring: 0: horizontal (vertical arrangement) (CIRAM A10 = PPU A11)
        /// |||||||              1: vertical (horizontal arrangement) (CIRAM A10 = PPU A10)
        /// ||||||+-- 1: Cartridge contains battery-backed PRG RAM ($6000-7FFF) or other persistent memory
        /// |||||+--- 1: 512-byte trainer at $7000-$71FF (stored before PRG data)
        /// ||||+---- 1: Ignore mirroring control or above mirroring bit; instead provide four-screen VRAM
        /// ++++----- Lower nibble of mapper number
        /// </summary>
        // clang-format off
        uint8_t m_Mapper1;

        /// <summary>
        /// Mapper, VS/Playchoice, NES 2.0
        /// </summary>
        uint8_t m_Mapper2;

        /// <summary>
        /// PRG-RAM size
        /// </summary>
        uint8_t m_PrgRamSize;

        /// <summary>
        /// TV system
        /// </summary>
        uint8_t m_TvSystem1;

        /// <summary>
        /// TV system, PRG-RAM presence
        /// </summary>
        uint8_t m_TvSystem2;

        /// <summary>
        /// Unused padding
        /// </summary>
        char m_Unused[5];
    };
    iNesHeader m_iNesHeader;

    MIRRORING_MODE m_MirroringMode = MIRRORING_MODE::HORIZONTAL;

};

}  // namespace dearnes
