// Copyright (c) 2020 Emmanuel Arias
#include "include/ppu_imgui_widgets.h"

#include <fmt/core.h>
#include <imgui.h>

#include "include/ppu.h"
#include "include/ui_config.h"

namespace cpuemulator {

PpuImguiWidget::PpuImguiWidget(const UiConfig& uiConfig, Ppu* ppuPtr)
    : m_UiConfig{uiConfig}, m_PpuPtr{ppuPtr} {}

void PpuImguiWidget::Update() {
    m_SpriteOutputScreen.CopyTextureFromArray(m_PpuPtr->GetOutputScreen());
    m_SpriteOutputScreen.Update();

    if (m_UiConfig.m_PpuShowPatternTable0) {
        UpdatePatternTableSprite(m_SpritePatternTables[0], 0, 0);
        m_SpritePatternTables[0].Update();
    }
    if (m_UiConfig.m_PpuShowPatternTable1) {
        UpdatePatternTableSprite(m_SpritePatternTables[1], 1, 0);
        m_SpritePatternTables[1].Update();
    }

    for (int p = 0; p < 8; ++p)  // For each palette
    {
        for (int s = 0; s < 4; ++s)  // For each index
        {
            const int coordX = (p > 3) ? s + 5 : s;
            const int coordY = p % 4;
            m_SpritePalette.SetPixel(coordX, coordY,
                                     m_PpuPtr->GetColorFromPalette(p, s));
        }
    }
    m_SpritePalette.Update();
}

void PpuImguiWidget::UpdatePatternTableSprite(Sprite& sprite,
                                              unsigned int index,
                                              uint8_t palette) {
    for (uint16_t nTileX = 0; nTileX < 16; ++nTileX) {
        for (uint16_t nTileY = 0; nTileY < 16; ++nTileY) {
            uint16_t offset = nTileY * 256 + nTileX * 16;

            for (uint16_t row = 0; row < 8; ++row) {
                uint8_t tileLSB =
                    m_PpuPtr->PpuRead(index * 0x1000 + offset + row + 0);
                uint8_t tileMSB =
                    m_PpuPtr->PpuRead(index * 0x1000 + offset + row + 8);
                for (uint16_t col = 0; col < 8; ++col) {
                    uint8_t pixel = static_cast<uint8_t>(tileLSB & 0b01) +
                                    static_cast<uint8_t>((tileMSB << 1) & 0b10);
                    tileLSB >>= 1;
                    tileMSB >>= 1;

                    sprite.SetPixel(
                        nTileX * 8 + (7 - col), nTileY * 8 + row,
                        m_PpuPtr->GetColorFromPalette(palette, pixel));
                }
            }
        }
    }
}

void PpuImguiWidget::Render() {
    m_SpriteOutputScreen.Render();
    if (m_UiConfig.m_PpuShowPatternTable0) {
        m_SpritePatternTables[0].Render();
    }
    if (m_UiConfig.m_PpuShowPatternTable1) {
        m_SpritePatternTables[1].Render();
    }
    m_SpritePalette.Render();
}

void PpuImguiWidget::RenderWidgets() {
    auto GetNametableString = [&](std::size_t nametableId) -> std::string {
        std::string nametableStr = "";

        for (int y = 0; y < 30; ++y) {
            for (int x = 0; x < 32; ++x) {
                nametableStr += fmt::format(
                    "{:02x} ", m_PpuPtr->m_Nametables[nametableId][y * 32 + x]);
            }
            nametableStr += '\n';
        }
        return nametableStr;
    };

    ImGui::Begin("Nametable #0");
    ImGui::Text(GetNametableString(0).c_str());
    ImGui::End();

    ImGui::Begin("Nametable #1");
    ImGui::Text(GetNametableString(1).c_str());
    ImGui::End();
}
}  // namespace cpuemulator