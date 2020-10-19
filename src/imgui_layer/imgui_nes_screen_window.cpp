// Copyright (c) 2020 Emmanuel Arias
#include "include/imgui_layer/imgui_nes_screen_window.h"
#include <imgui.h>

#include "include/global_nes.h"
#include "include/ppu.h"

void ImGuiNesScreenWindow::Render() {
    if (!m_Show) {
        return;
    }
    if (!Begin(m_WindowName)) {
        End();
        return;
    }
    ImVec2 windowsSize = ImGui::GetContentRegionAvail();
    if (windowsSize.x < windowsSize.y) {
        m_NesScreenWidget.ScaleImageToWidth(windowsSize.x);
    } else {
        m_NesScreenWidget.ScaleImageToHeight(windowsSize.y);
    }
    m_NesScreenWidget.Render();
    End();
}

void ImGuiNesScreenWindow::Update() {
    if (m_Show) {
        m_NesScreenWidget.CopyTextureFromArray(
            g_GetGlobalNes()->m_Ppu->GetOutputScreen());
        m_NesScreenWidget.Update();
    }
}
