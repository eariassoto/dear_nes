// Copyright (c) 2020 Emmanuel Arias
#include "include/screen_widget.h"
#include <imgui.h>

#include "dear_nes_lib/nes.h"
#include "dear_nes_lib/ppu.h"
#include "include/dearnes_base_widget.h"

ScreenWidget::ScreenWidget(dearnes::Nes* nesPtr) : DearNESBaseWidget(nesPtr) {}

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
            m_NesPtr->GetPpu()->GetOutputScreen());
        m_NesScreenTextureImage.Update();
    }
}
