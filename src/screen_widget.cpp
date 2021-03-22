// Copyright (c) 2020 Emmanuel Arias
#include "include/screen_widget.h"
#include <imgui.h>

#include "include/global_nes.h"
#include "dear_nes_lib/ppu.h"

void ScreenWidget::Render() {
    if (!m_Show) {
        return;
    }
    if (!Begin(m_WindowName)) {
        End();
        return;
    }
    ImVec2 windowsSize = ImGui::GetContentRegionAvail();
    if (windowsSize.x < windowsSize.y) {
        m_NesScreenTextureImage.ScaleImageToWidth(windowsSize.x);
    } else {
        m_NesScreenTextureImage.ScaleImageToHeight(windowsSize.y);
    }
    m_NesScreenTextureImage.Render();
    End();
}

void ScreenWidget::Update(float delta) {
    if (m_Show) {
        m_NesScreenTextureImage.CopyTextureFromArray(
            g_GetGlobalNes()->GetPpu()->GetOutputScreen());
        m_NesScreenTextureImage.Update();
    }
}
