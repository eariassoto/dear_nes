// Copyright (c) 2020 Emmanuel Arias
#include "include/ppu.h"
#include "include/cartridge.h"
#include "include/logger.h"

namespace cpuemulator {

int Ppu::GetColorFromPalette(uint8_t palette, uint8_t pixel) {
    uint8_t data = PpuRead(0x3F00 + (palette << 4) + pixel);
    return m_PalScreen[data];
}

uint8_t Ppu::CpuRead(uint16_t address, bool readOnly) {
    uint8_t data = 0x00;
    switch (address) {
        case 0x0000:  // control
            break;
        case 0x0001:  // mask
            break;
        case 0x0002:  // Status
            data =
                (m_StatusReg.GetRegister() & 0xE0) | (m_PpuDataBuffer & 0x1F);
            m_StatusReg.SetField(StatusRegisterFields::VERTICAL_BLANK, false);
            m_AddressLatch = 0x00;
            break;
        case 0x0003:  // OAM address
            break;
        case 0x0004:  // OAM data
            break;
        case 0x0005:  // Scroll
            break;
        case 0x0006:  // PPU address
            break;
        case 0x0007:  // PPU data
            data = m_PpuDataBuffer;
            m_PpuDataBuffer = PpuRead(m_VramAddress.reg);

            if (m_VramAddress.reg > 0x3F00) {
                data = m_PpuDataBuffer;
            }
            m_VramAddress.reg += m_ControlReg.GetField(INCREMENT_MODE) ? 32 : 1;
            break;
        default:
            break;
    }
    return data;
}

void Ppu::CpuWrite(uint16_t address, uint8_t data) {
    switch (address) {
        case 0x0000:  // control
            m_ControlReg.SetRegister(data);
            m_TramAddress.nametable_x =
                m_ControlReg.GetField(ControlRegisterFields::NAMETABLE_X);
            m_TramAddress.nametable_y =
                m_ControlReg.GetField(ControlRegisterFields::NAMETABLE_Y);
            break;
        case 0x0001:  // mask
            m_MaskReg.SetRegister(data);
            break;
        case 0x0002:  // Status
            break;
        case 0x0003:  // OAM address
            break;
        case 0x0004:  // OAM data
            break;
        case 0x0005:  // Scroll
            if (m_AddressLatch == 0x00) {
                fine_x = data & 0x07;
                m_TramAddress.coarse_x = data >> 3;
                m_AddressLatch = 0x01;
            } else {
                m_TramAddress.fine_y = data & 0x07;
                m_TramAddress.coarse_y = data >> 3;

                m_AddressLatch = 0x00;
            }
            break;
        case 0x0006:  // PPU address
            if (m_AddressLatch == 0x00) {
                m_TramAddress.reg = (m_TramAddress.reg & 0x00FF) |
                                    (static_cast<uint16_t>(data) << 8);
                m_AddressLatch = 0x01;
            } else {
                m_TramAddress.reg = (m_TramAddress.reg & 0xFF00) | data;
                m_VramAddress = m_TramAddress;
                m_AddressLatch = 0x00;
            }
            break;
        case 0x0007:  // PPU data
            PpuWrite(m_TramAddress.reg, data);
            m_TramAddress.reg += m_ControlReg.GetField(INCREMENT_MODE) ? 32 : 1;
            break;
        default:
            break;
    }
}

uint8_t Ppu::PpuRead(uint16_t address, bool readOnly) {
    uint8_t data = 0x00;
    address &= 0x3FFF;

    if (m_Cartridge->PpuRead(address, data)) {
    } else if (address >= 0x0000 && address <= 0x1FFF) {
        data = m_TablePattern[(address & 0x1000) >> 12][address & 0x0FFF];
    } else if (address >= 0x2000 && address <= 0x3EFF) {
        address &= 0x0FFF;
        if (m_Cartridge->mirror == Cartridge::MIRROR::VERTICAL) {
            // Vertical
            if (address >= 0x0000 && address <= 0x03FF)
                data = m_TableName[0][address & 0x03FF];
            if (address >= 0x0400 && address <= 0x07FF)
                address = m_TableName[1][address & 0x03FF];
            if (address >= 0x0800 && address <= 0x0BFF)
                data = m_TableName[0][address & 0x03FF];
            if (address >= 0x0C00 && address <= 0x0FFF)
                data = m_TableName[1][address & 0x03FF];
        } else if (m_Cartridge->mirror == Cartridge::MIRROR::HORIZONTAL) {
            // Horizontal
            if (address >= 0x0000 && address <= 0x03FF)
                data = m_TableName[0][address & 0x03FF];
            if (address >= 0x0400 && address <= 0x07FF)
                data = m_TableName[0][address & 0x03FF];
            if (address >= 0x0800 && address <= 0x0BFF)
                data = m_TableName[1][address & 0x03FF];
            if (address >= 0x0C00 && address <= 0x0FFF)
                data = m_TableName[1][address & 0x03FF];
        }
    } else if (address >= 0x3F00 && address <= 0x3FFF) {
        address &= 0x001F;
        if (address == 0x0010) address = 0x0000;
        if (address == 0x0014) address = 0x0004;
        if (address == 0x0018) address = 0x0008;
        if (address == 0x001C) address = 0x000C;
        data = m_TablePalette[address];
    }
    return data;
}

void Ppu::PpuWrite(uint16_t address, uint8_t data) {
    address &= 0x3FFF;
    if (m_Cartridge->PpuWrite(address, data)) {
    } else if (address >= 0x0000 && address <= 0x1FFF) {
        m_TablePattern[(address & 0x1000) >> 12][address & 0x0FFF] = data;
    } else if (address >= 0x2000 && address <= 0x3EFF) {
        address &= 0x0FFF;
        if (m_Cartridge->mirror == Cartridge::MIRROR::VERTICAL) {
            // Vertical
            if (address >= 0x0000 && address <= 0x03FF)
                m_TableName[0][address & 0x03FF] = data;
            if (address >= 0x0400 && address <= 0x07FF)
                m_TableName[1][address & 0x03FF] = data;
            if (address >= 0x0800 && address <= 0x0BFF)
                m_TableName[0][address & 0x03FF] = data;
            if (address >= 0x0C00 && address <= 0x0FFF)
                m_TableName[1][address & 0x03FF] = data;
        } else if (m_Cartridge->mirror == Cartridge::MIRROR::HORIZONTAL) {
            // Horizontal
            if (address >= 0x0000 && address <= 0x03FF)
                m_TableName[0][address & 0x03FF] = data;
            if (address >= 0x0400 && address <= 0x07FF)
                m_TableName[0][address & 0x03FF] = data;
            if (address >= 0x0800 && address <= 0x0BFF)
                m_TableName[1][address & 0x03FF] = data;
            if (address >= 0x0C00 && address <= 0x0FFF)
                m_TableName[1][address & 0x03FF] = data;
        }
    } else if (address >= 0x3F00 && address <= 0x3FFF) {
        address &= 0x001F;
        if (address == 0x0010) address = 0x0000;
        if (address == 0x0014) address = 0x0004;
        if (address == 0x0018) address = 0x0008;
        if (address == 0x001C) address = 0x000C;
        m_TablePalette[address] = data;
    }
}

void Ppu::ConnectCatridge(const std::shared_ptr<Cartridge>& cartridge) {
    Logger::Get().Log("PPU", "Connecting cartridge");
    m_Cartridge = cartridge;
}

void Ppu::Clock() {
    auto IncrementScrollX = [&]() {
        if (m_MaskReg.GetField(MaskRegisterFields::RENDER_BACKGROUND) ||
            m_MaskReg.GetField(MaskRegisterFields::RENDER_SPRITES)) {
            if (m_VramAddress.coarse_x == 31) {
                m_VramAddress.coarse_x = 0;
                m_VramAddress.nametable_x = ~m_VramAddress.nametable_x;
            } else {
                m_VramAddress.coarse_x++;
            }
        }
    };

    auto IncrementScrollY = [&]() {
        if (m_MaskReg.GetField(MaskRegisterFields::RENDER_BACKGROUND) ||
            m_MaskReg.GetField(MaskRegisterFields::RENDER_SPRITES)) {
            if (m_VramAddress.fine_y < 7) {
                m_VramAddress.fine_y++;
            } else {
                m_VramAddress.fine_y = 0;

                if (m_VramAddress.coarse_y == 29) {
                    m_VramAddress.coarse_y = 0;
                    m_VramAddress.nametable_y = ~m_VramAddress.nametable_y;
                } else if (m_VramAddress.coarse_y == 31) {
                    m_VramAddress.coarse_y = 0;
                } else {
                    m_VramAddress.coarse_y++;
                }
            }
        }
    };

    auto TransferAddressX = [&]() {
        if (m_MaskReg.GetField(MaskRegisterFields::RENDER_BACKGROUND) ||
            m_MaskReg.GetField(MaskRegisterFields::RENDER_SPRITES)) {
            m_VramAddress.nametable_x = m_TramAddress.nametable_x;
            m_VramAddress.coarse_x = m_TramAddress.coarse_x;
        }
    };

    auto TransferAddressY = [&]() {
        if (m_MaskReg.GetField(MaskRegisterFields::RENDER_BACKGROUND) ||
            m_MaskReg.GetField(MaskRegisterFields::RENDER_SPRITES)) {
            m_VramAddress.fine_y = m_TramAddress.fine_y;
            m_VramAddress.nametable_y = m_TramAddress.nametable_y;
            m_VramAddress.coarse_y = m_TramAddress.coarse_y;
        }
    };

    auto LoadBackgroundShifters = [&]() {
        bgShifterPatternLo = (bgShifterPatternLo & 0xFF00) | bgNextTileLsb;
        bgShifterPatternHi = (bgShifterPatternHi & 0xFF00) | bgNextTileMsb;

        bgShifterAttribLo = (bgShifterAttribLo & 0xFF00) |
                            ((bgNextTileAttribute & 0b01) ? 0xFF : 0x00);
        bgShifterAttribHi = (bgShifterAttribHi & 0xFF00) |
                            ((bgNextTileAttribute & 0b10) ? 0xFF : 0x00);
    };

    auto UpdateShifters = [&]() {
        if (m_MaskReg.GetField(MaskRegisterFields::RENDER_BACKGROUND)) {
            bgShifterPatternLo <<= 1;
            bgShifterPatternHi <<= 1;

            bgShifterAttribLo <<= 1;
            bgShifterAttribHi <<= 1;
        }
    };

    if (m_ScanLine >= -1 && m_ScanLine < 240) {
        if (m_ScanLine == 0 && m_Cycle == 0) {
            // "Odd Frame" cycle skip
            m_Cycle = 1;
        }

        if (m_ScanLine == -1 && m_Cycle == 1) {
            m_StatusReg.SetField(StatusRegisterFields::VERTICAL_BLANK, false);
        }

        if ((m_Cycle >= 2 && m_Cycle < 258) ||
            (m_Cycle >= 321 && m_Cycle < 338)) {
            UpdateShifters();

            switch ((m_Cycle - 1) % 8) {
                case 0:
                    LoadBackgroundShifters();
                    bgNextTileId =
                        PpuRead(0x2000 | (m_VramAddress.reg & 0x0FFF));
                    break;
                case 2:
                    bgNextTileAttribute =
                        PpuRead(0x23C0 | (m_VramAddress.nametable_y << 11) |
                                (m_VramAddress.nametable_x << 10) |
                                ((m_VramAddress.coarse_y >> 2) << 3) |
                                (m_VramAddress.coarse_x >> 2));
                    if (m_VramAddress.coarse_y & 0x02)
                        bgNextTileAttribute >>= 4;
                    if (m_VramAddress.coarse_x & 0x02)
                        bgNextTileAttribute >>= 2;
                    bgNextTileAttribute &= 0x03;
                    break;
                case 4:
                    bgNextTileLsb =
                        PpuRead((m_ControlReg.GetField(
                                     ControlRegisterFields::PATTERN_BACKGROUND)
                                 << 12) +
                                ((uint16_t)bgNextTileId << 4) +
                                (m_VramAddress.fine_y + 0));
                    break;
                case 6:
                    bgNextTileMsb =
                        PpuRead((m_ControlReg.GetField(
                                     ControlRegisterFields::PATTERN_BACKGROUND)
                                 << 12) +
                                ((uint16_t)bgNextTileId << 4) +
                                (m_VramAddress.fine_y + 8));
                    break;
                case 7:
                    IncrementScrollX();
                    break;
            }
        }

        if (m_Cycle == 256) {
            IncrementScrollY();
        }
        if (m_Cycle == 257) {
            LoadBackgroundShifters();
            TransferAddressX();
        }

		if (m_Cycle == 338 || m_Cycle == 340) {
            bgNextTileId = PpuRead(0x2000 | (m_VramAddress.reg & 0x0FFF));
        }

        if (m_ScanLine == -1 && m_Cycle >= 280 && m_Cycle < 305) {
            TransferAddressY();
        }
    }

    if (m_ScanLine == 241 && m_Cycle == 1) {
        m_StatusReg.SetField(StatusRegisterFields::VERTICAL_BLANK, true);
        if (m_ControlReg.GetField(ControlRegisterFields::ENABLE_NMI)) {
            m_DoNMI = true;
        }
    }

    if (m_MaskReg.GetField(MaskRegisterFields::RENDER_BACKGROUND)) {
        uint16_t bitMux = 0x8000 >> fine_x;

        uint8_t p0_pixel = (bgShifterPatternLo & bitMux) > 0;
        uint8_t p1_pixel = (bgShifterPatternHi & bitMux) > 0;

        uint8_t bgPixel = (p1_pixel << 1) | p0_pixel;

        uint8_t bg_pal0 = (bgShifterAttribLo & bitMux) > 0;
        uint8_t bg_pal1 = (bgShifterAttribHi & bitMux) > 0;
        uint8_t bgPalette = (bg_pal1 << 1) | bg_pal0;

		m_SpriteScreen.SetPixel(m_Cycle - 1, m_ScanLine,
                           GetColorFromPalette(bgPalette, bgPixel));
    }

    // int color = (rand() % 2) ? 0xFFFFFFFF : 0xFF000000;
    // m_SpriteScreen.SetPixel(m_Cycle - 1, m_ScanLine, color);

    ++m_Cycle;
    if (m_Cycle >= 341) {
        m_Cycle = 0;
        ++m_ScanLine;
        if (m_ScanLine >= 261) {
            m_ScanLine = -1;
            isFrameComplete = true;
        }
    }
}

Sprite& Ppu::GetPatternTable(unsigned int index, uint8_t palette) {
    for (uint16_t nTileX = 0; nTileX < 16; ++nTileX) {
        for (uint16_t nTileY = 0; nTileY < 16; ++nTileY) {
            uint16_t offset = nTileY * 256 + nTileX * 16;

            for (uint16_t row = 0; row < 8; ++row) {
                uint8_t tileLSB = PpuRead(index * 0x1000 + offset + row + 0);
                uint8_t tileMSB = PpuRead(index * 0x1000 + offset + row + 8);
                for (uint16_t col = 0; col < 8; ++col) {
                    uint8_t pixel = static_cast<uint8_t>(tileLSB & 0b01) +
                                    static_cast<uint8_t>((tileMSB << 1) & 0b10);
                    tileLSB >>= 1;
                    tileMSB >>= 1;

                    m_SpritePatternTable[index].SetPixel(
                        nTileX * 8 + (7 - col), nTileY * 8 + row,
                        GetColorFromPalette(palette, pixel));
                }
            }
        }
    }
    // todo check bounds error
    return m_SpritePatternTable[index];
}

}  // namespace cpuemulator