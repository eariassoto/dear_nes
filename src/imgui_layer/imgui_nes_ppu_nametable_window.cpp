// Copyright (c) 2020 Emmanuel Arias
#include "include/imgui_layer/imgui_nes_ppu_nametable_window.h"

#include <fmt/core.h>
#include <imgui.h>

#include "include/global_nes.h"
#include "include/nes.h"
#include "include/ppu.h"

ImGuiNesPpuNametableWindow::ImGuiNesPpuNametableWindow(
    unsigned int nametableIdx)
    : m_NametableIdx{nametableIdx},
      m_WindowName{fmt::format("Ppu Nametable {}", m_NametableIdx)} {}

void ImGuiNesPpuNametableWindow::Render() {
    if (!m_Show) {
        return;
    }
    if (!Begin(m_WindowName)) {
        End();
        return;
    }
    cpuemulator::Nes* nesEmulator = g_GetGlobalNes();
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

