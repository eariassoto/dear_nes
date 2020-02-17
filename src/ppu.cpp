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
            m_PpuDataBuffer = PpuRead(m_PpuAddress);

            if (m_PpuAddress > 0x3F00) {
                data = m_PpuDataBuffer;
            }
            m_PpuAddress += m_ControlReg.GetField(INCREMENT_MODE) ? 32 : 1;
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
            break;
        case 0x0006:  // PPU address
            if (m_AddressLatch == 0x00) {
                m_PpuAddress = (m_PpuAddress & 0x00FF) |
                               (static_cast<uint16_t>(data) << 8);
                m_AddressLatch = 0x01;
            } else {
                m_PpuAddress = (m_PpuAddress & 0xFF00) | data;
                m_AddressLatch = 0x00;
            }
            break;
        case 0x0007:  // PPU data
            PpuWrite(m_PpuAddress, data);
            m_PpuAddress += m_ControlReg.GetField(INCREMENT_MODE) ? 32 : 1;
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
    if (m_ScanLine == -1 && m_Cycle == 1) {
        m_StatusReg.SetField(StatusRegisterFields::VERTICAL_BLANK, false);
    }
    if (m_ScanLine == 241 && m_Cycle == 1) {
        m_StatusReg.SetField(StatusRegisterFields::VERTICAL_BLANK, true);
        if (m_ControlReg.GetField(ControlRegisterFields::ENABLE_NMI)) {
            m_DoNMI = true;
        }
    }

    int color = (rand() % 2) ? 0xFFFFFFFF : 0xFF000000;
    m_SpriteScreen.SetPixel(m_Cycle - 1, m_ScanLine, color);

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