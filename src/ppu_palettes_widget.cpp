// Copyright (c) 2020-2021 Emmanuel Arias
#include "include/ppu_palettes_widget.h"

#include <imgui.h>

#include "dear_nes_lib/nes.h"
#include "dear_nes_lib/ppu.h"
#include "include/dearnes_base_widget.h"

PpuPalettesWidget::PpuPalettesWidget(dearnes::Nes* nesPtr)
    : DearNESBaseWidget(nesPtr)
    , m_WindowName{"Ppu Palettes"} {}

void PpuPalettesWidget::Render() {
    if (!m_Show) {
        return;
    }
    if (!Begin(m_WindowName)) {
        End();
        return;
    }

    ImVec2 windowsSize = ImGui::GetContentRegionAvail();
    if (windowsSize.x < windowsSize.y) {
        m_PpuBackgroundPalette.ScaleImageToWidth(windowsSize.x);
        m_PpuCharacterPalette.ScaleImageToWidth(windowsSize.x);
    } else {
        m_PpuBackgroundPalette.ScaleImageToHeight(windowsSize.y);
        m_PpuCharacterPalette.ScaleImageToHeight(windowsSize.y);
    }

    m_PpuBackgroundPalette.Render();
    ImGui::Separator();
    m_PpuCharacterPalette.Render();

    End();
}

void PpuPalettesWidget::Update(float /*delta*/) {
    if (m_Show) {
        UpdatePatternTable();
        m_PpuBackgroundPalette.Update();
        m_PpuCharacterPalette.Update();
    }
}

void PpuPalettesWidget::UpdatePatternTable() {
    auto ppuPtr = m_NesPtr->GetPpu();
    for (uint8_t paletteIdx = 0; paletteIdx < 4; ++paletteIdx) {
        for (uint8_t colorIdx = 0; colorIdx < 4; ++colorIdx) {
            m_PpuBackgroundPalette.SetPixel(
                colorIdx, paletteIdx,
                ppuPtr->GetColorFromPalette(paletteIdx, colorIdx));
            m_PpuCharacterPalette.SetPixel(
                colorIdx, paletteIdx,
                ppuPtr->GetColorFromPalette(4 + paletteIdx, colorIdx));
        }
    }
}
