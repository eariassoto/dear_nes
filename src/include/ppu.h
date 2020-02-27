// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <cstdint>
#include <memory>

#include "include/sprite.h"

namespace cpuemulator {

    class Cartridge;
    class Sprite;

    template <typename RegEnumType>
    struct PpuRegister {
        bool GetField(RegEnumType field) const {
            const int index = static_cast<int>(field);
            return (m_Register >> index) & 0x01;
        }

        void SetField(RegEnumType field, bool value) {
            const int index = static_cast<int>(field);
            if (value) {
                m_Register |= 0x01 << index;
            }
            else {
                m_Register &= ~(0x01 << index);
            }
        }

        uint8_t GetRegister() const { return m_Register; }
        void SetRegister(uint8_t value) { m_Register = value; }

    private:
        uint8_t m_Register = 0x00;
    };

    enum StatusRegisterFields {
        SPRITE_OVERFLOW = 5,
        SPRITE_ZERO_HIT = 6,
        VERTICAL_BLANK = 7
    };

    enum MaskRegisterFields {
        GRAYSCALE = 0,
        RENDER_BACKGROUND_LEFT = 1,
        RENDER_SPRITES_LEFT = 2,
        RENDER_BACKGROUND = 3,
        RENDER_SPRITES = 4,
        ENHANCE_RED = 5,
        ENHANCE_GREEN = 6,
        ENHANCE_BLUE = 7
    };

    enum ControlRegisterFields {
        NAMETABLE_X = 0,
        NAMETABLE_Y = 1,
        INCREMENT_MODE = 2,
        PATTERN_SPRITE = 3,
        PATTERN_BACKGROUND = 4,
        SPRITE_SIZE = 5,
        SLAVE_MODE = 6,  // unused
        ENABLE_NMI = 7
    };

    union LoopyRegister {
        // Credit to Loopy for working this out :D
        struct {
            uint16_t coarse_x : 5;
            uint16_t coarse_y : 5;
            uint16_t nametable_x : 1;
            uint16_t nametable_y : 1;
            uint16_t fine_y : 3;
            uint16_t unused : 1;
        };

        uint16_t reg = 0x0000;
    };

    class Ppu {
    public:
        void Update();
        void Render();

        uint8_t CpuRead(uint16_t address, bool readOnly = false);

        void CpuWrite(uint16_t address, uint8_t data);

        void ConnectCatridge(const std::shared_ptr<Cartridge>& cartridge);
        void Clock();

        void UpdatePatternTableSprite(Sprite& sprite, unsigned int index,
            uint8_t palette);

        // TODO: This should be private
        bool isFrameComplete = false; 
        
        // TODO Made this private and provide callbacks, or do post update
        bool m_DoNMI = false;

        // TODO: Provide Api Call
        uint8_t* m_OAMPtr = (uint8_t*)m_OAM;

    private:

        int GetColorFromPalette(uint8_t palette, uint8_t pixel);

        uint8_t PpuRead(uint16_t address, bool readOnly = false);

        void PpuWrite(uint16_t address, uint8_t data);

        /// PPU Nametables
        /// A nametable is a 1024 byte area of memory used by the PPU
        /// to lay out backgrounds. Each byte in the nametable controls
        /// one 8x8 pixel character cell, and each nametable has 30 rows of 32 tiles
        /// each, for 960 ($3C0) bytes; the rest is used by each nametable's
        /// attribute table. With each tile being 8x8 pixels, this makes a total of
        /// 256x240 pixels in one map, the same size as one full screen.
        /// https://wiki.nesdev.com/w/index.php/PPU_nametables
        uint8_t m_Nametables[2][1024] = { 0 };

        /// The palette for the background runs from VRAM $3F00 to $3F0F; the
        /// palette for the sprites runs from $3F10 to $3F1F. Each color takes up
        /// one byte. https://wiki.nesdev.com/w/index.php/PPU_palettes
        uint8_t m_PaletteTable[32] = { 0 };

        /// The pattern table is an area of memory connected to the PPU that defines
        /// the shapes of tiles that make up backgrounds and sprites. Each tile in
        /// the pattern table is 16 bytes, made of two planes. The first plane
        /// controls bit 0 of the color; the second plane controls bit 1.
        /// https://wiki.nesdev.com/w/index.php/PPU_pattern_tables
        uint8_t m_PatternTables[2][4096] = { 0 };

        Sprite m_SpriteOutputScreen = Sprite{ "NES Screen", 256, 240, 2, 10, 10 };

        Sprite m_SpritePatternTables[2] = {
            Sprite{"Pattern Table #0", 128, 128, 2, 542, 10},
            Sprite{"Pattern Table #1", 128, 128, 2, 818, 10} };

        Sprite m_SpritePalette{ "Color Palettes", 9, 4, 30, 542, 310 };

        std::shared_ptr<Cartridge> m_Cartridge;

        int16_t m_ScanLine = 0;
        int16_t m_Cycle = 0;

        PpuRegister<StatusRegisterFields> m_StatusReg;
        PpuRegister<MaskRegisterFields> m_MaskReg;
        PpuRegister<ControlRegisterFields> m_ControlReg;
        uint8_t m_AddressLatch = 0x00;
        uint8_t m_PpuDataBuffer = 0x00;

        LoopyRegister m_VramAddress;
        LoopyRegister m_TramAddress;

        uint8_t fine_x = 0x00;

        uint8_t bgNextTileId = 0x00;
        uint8_t bgNextTileAttribute = 0x00;
        uint8_t bgNextTileLsb = 0x00;
        uint8_t bgNextTileMsb = 0x00;

        uint16_t bgShifterPatternLo = 0x0000;
        uint16_t bgShifterPatternHi = 0x0000;
        uint16_t bgShifterAttribLo = 0x0000;
        uint16_t bgShifterAttribHi = 0x0000;

        struct ObjectAttributeEntry {
            uint8_t y;
            uint8_t id;
            uint8_t attribute;
            uint8_t x;
        };
        ObjectAttributeEntry m_OAM[64];

        uint8_t m_OAMAddress = 0x00;

        ObjectAttributeEntry m_SpriteScanLine[8];
        uint8_t m_SpriteCount = 0;

        uint8_t m_SpriteShifterPatternLo[8];
        uint8_t m_SpriteShifterPatternHi[8];

        bool m_SpriteZeroHitPossible = false;
        bool m_SpriteZeroBeingRendered = false;

        // Colors are in format ARGB
        // Table taken from https://wiki.nesdev.com/w/index.php/PPU_palettes
        static constexpr unsigned int m_PalScreen[0x40] = {
            0xFF545454, 0xFF001E74, 0xFF081090, 0xFF300088, 0xFF440064, 0xFF5C0030,
            0xFF540400, 0xFF3C1800, 0xFF202A00, 0xFF083A00, 0xFF004000, 0xFF003C00,
            0xFF00323C, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF989698, 0xFF084CC4,
            0xFF3032EC, 0xFF5C1EE4, 0xFF8814B0, 0xFFA01464, 0xFF982220, 0xFF783C00,
            0xFF545A00, 0xFF287200, 0xFF087C00, 0xFF007628, 0xFF006678, 0xFF000000,
            0xFF000000, 0xFF000000, 0xFFECEEEC, 0xFF4C9AEC, 0xFF787CEC, 0xFFB062EC,
            0xFFE454EC, 0xFFEC58B4, 0xFFEC6A64, 0xFFD48820, 0xFFA0AA00, 0xFF74C400,
            0xFF4CD020, 0xFF38CC6C, 0xFF38B4CC, 0xFF3C3C3C, 0xFF000000, 0xFF000000,
            0xFFECEEEC, 0xFFA8CCEC, 0xFFBCBCEC, 0xFFD4B2EC, 0xFFECAEEC, 0xFFECAED4,
            0xFFECD4AE, 0xFFE4C490, 0xFFCCD278, 0xFFB4DE78, 0xFFA8E290, 0xFF98E2B4,
            0xFFA0D6E4, 0xFFA0A2A0, 0xFF000000, 0xFF000000 };
    };

}  // namespace cpuemulator
