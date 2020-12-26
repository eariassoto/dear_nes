// Copyright (c) 2020 Emmanuel Arias
#include "include/ppu_nametable_widget.h"

#include <fmt/core.h>
#include <imgui.h>

#include "include/global_nes.h"
#include "dear_nes_lib/nes.h"
#include "dear_nes_lib/ppu.h"

PpuNametableWidget::PpuNametableWidget(
    unsigned int nametableIdx)
    : m_NametableIdx{nametableIdx},
      m_WindowName{fmt::format("Ppu Nametable {}", m_NametableIdx)} {}

void PpuNametableWidget::Render() {
    if (!m_Show) {
        return;
    }
    if (!Begin(m_WindowName)) {
        End();
        return;
    }
    dearnes::Nes* nesEmulator = g_GetGlobalNes();
    auto m_PpuPtr = nesEmulator->GetPpu();
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

    ImGui::Text(GetNametableString(m_NametableIdx).c_str());
    End();
}

