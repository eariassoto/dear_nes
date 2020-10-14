
// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <cstdint>
//#include <functional>
//#include <memory>
//#include <array>

#include "include/sprite.h"
//#include "include/ui_config.h"

namespace cpuemulator {

// class Cartridge;
class Sprite;
struct UiConfig;
class Ppu;

class PpuImguiWidget {
   public:
    PpuImguiWidget(const UiConfig& uiConfig, Ppu* ppuPtr);

    void Update();
    void Render();
    void RenderWidgets();

   private:
    void UpdatePatternTableSprite(Sprite& sprite, unsigned int index,
                                  uint8_t palette);

    const UiConfig& m_UiConfig;
    Ppu* m_PpuPtr;

    Sprite m_SpriteOutputScreen = Sprite{"NES Screen", 256, 240, 2, 10, 10};

    Sprite m_SpritePatternTables[2] = {
        Sprite{"Pattern Table #0", 128, 128, 2, 542, 10},
        Sprite{"Pattern Table #1", 128, 128, 2, 818, 10}};

    Sprite m_SpritePalette{"Color Palettes", 9, 4, 30, 542, 310};
};

}  // namespace cpuemulator
