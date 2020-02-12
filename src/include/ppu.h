// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <cstdint>
#include <memory>
#include "include/sprite.h"

namespace cpuemulator {

class Cartridge;
class Sprite;

class Ppu {
   private:
    uint8_t m_TableName[2][1024];
    uint8_t m_TablePalette[32];

   public:
    uint8_t CpuRead(uint16_t address, bool readOnly = false);
    void CpuWrite(uint16_t address, uint8_t data);

    uint8_t PpuRead(uint16_t address, bool readOnly = false);
    void PpuWrite(uint16_t address, uint8_t data);

   private:
    std::shared_ptr<Cartridge> m_Cartridge;

   public:
    void ConnectCatridge(const std::shared_ptr<Cartridge>& cartridge);
    void Clock();

public:
    int palScreen[40];
    Sprite m_SpriteScreen = Sprite{ 256, 240, 2, 10, 10 };
    Sprite m_SpriteNameTable[2] = { Sprite{ 256, 240, 2, 10, 10 }, Sprite{ 256, 240, 2, 10, 10 } };
    Sprite m_SpritePatternTable[2] = { Sprite{ 128, 128, 2, 10, 10 }, Sprite{ 128, 128, 2, 10, 10 } };
    bool isFrameComplete = false;

private:
    int16_t m_ScanLine = 0;
    int16_t m_Cycle = 0;
};
}  // namespace cpuemulator
