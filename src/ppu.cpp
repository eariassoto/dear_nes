// Copyright (c) 2020 Emmanuel Arias
#include "include/ppu.h"
#include "include/cartridge.h"
#include "include/logger.h"

namespace cpuemulator {
uint8_t Ppu::CpuRead(uint16_t address, bool readOnly) {
    uint8_t data = 0x00;
    switch (address) {
        case 0x0000:  // control
            break;
        case 0x0001:  // mask
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
            break;
        case 0x0007:  // PPU data
            break;
        default:
            break;
    }
    return data;
}

void Ppu::CpuWrite(uint16_t address, uint8_t data) {
    switch (address) {
        case 0x0000:  // control
            break;
        case 0x0001:  // mask
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
            break;
        case 0x0007:  // PPU data
            break;
        default:
            break;
    }
}

uint8_t Ppu::PpuRead(uint16_t address, bool readOnly) {
    uint8_t data = 0x00;
    address &= 0x3FFF;

	if (m_Cartridge->PpuRead(address, data))
	{

	}
    return data;
}

void Ppu::PpuWrite(uint16_t address, uint8_t data) { 
	address &= 0x3FFF;
	if (m_Cartridge->PpuWrite(address, data))
	{

	}
}

void Ppu::ConnectCatridge(const std::shared_ptr<Cartridge>& cartridge) {
    Logger::Get().Log("PPU", "Connecting cartridge");
    m_Cartridge = cartridge;
}

void Ppu::Clock()
{
    int color = (rand() % 2) ? 0x00FFFFFF : 0x00000000;
    m_SpriteScreen.SetPixel(m_Cycle - 1, m_ScanLine, color);

    ++m_Cycle;
    if (m_Cycle >= 341)
    {
        m_Cycle = 0;
        ++m_ScanLine;
        if (m_ScanLine >= 261)
        {
            m_ScanLine = -1;
            isFrameComplete = true;
        }
    }
}

}  // namespace cpuemulator