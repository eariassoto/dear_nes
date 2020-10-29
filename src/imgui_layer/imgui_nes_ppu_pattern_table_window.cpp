// Copyright (c) 2020 Emmanuel Arias
#include "include/imgui_layer/imgui_nes_ppu_pattern_table_window.h"

#include <fmt/core.h>
#include <imgui.h>

#include "include/global_nes.h"
#include "virtual-nes/nes.h"
#include "virtual-nes/ppu.h"

ImGuiNesPpuPatternTableWindow::ImGuiNesPpuPatternTableWindow(
    unsigned int patternTableIdx)
    : m_PatternTableIdx{patternTableIdx},
      m_WindowName{fmt::format("Ppu Pattern Table {}", m_PatternTableIdx)} {}

void ImGuiNesPpuPatternTableWindow::Render() {
    if (!m_Show) {
        return;
    }
    if (!Begin(m_WindowName)) {
        End();
        return;
    }
    ImVec2 windowsSize = ImGui::GetContentRegionAvail();
    if (windowsSize.x < windowsSize.y) {
        m_PpuPatternTable.ScaleImageToWidth(windowsSize.x);
    } else {
        m_PpuPatternTable.ScaleImageToHeight(windowsSize.y);
    }
    m_PpuPatternTable.Render();
    End();
}

void ImGuiNesPpuPatternTableWindow::Update() {
    if (m_Show) {
        UpdatePatternTable();
        m_PpuPatternTable.Update();
    }
}

void ImGuiNesPpuPatternTableWindow::UpdatePatternTable() {
    virtualnes::Nes* nesEmulator = g_GetGlobalNes();
    auto ppuPtr = nesEmulator->GetPpu();
    for (uint16_t nTileX = 0; nTileX < 16; ++nTileX) {
        for (uint16_t nTileY = 0; nTileY < 16; ++nTileY) {
            uint16_t offset = nTileY * 256 + nTileX * 16;

            for (uint16_t row = 0; row < 8; ++row) {
                uint8_t tileLSB = ppuPtr->PpuRead(m_PatternTableIdx * 0x1000 +
                                                    offset + row + 0);
                uint8_t tileMSB = ppuPtr->PpuRead(m_PatternTableIdx * 0x1000 +
                                                    offset + row + 8);
                for (uint16_t col = 0; col < 8; ++col) {
                    uint8_t pixel = static_cast<uint8_t>(tileLSB & 0b01) +
                                    static_cast<uint8_t>((tileMSB << 1) & 0b10);
                    tileLSB >>= 1;
                    tileMSB >>= 1;

                    m_PpuPatternTable.SetPixel(
                        nTileX * 8 + (7 - col), nTileY * 8 + row,
                        ppuPtr->GetColorFromPalette(DEFAULT_PALETTE, pixel));
                }
            }
        }
    }
}
