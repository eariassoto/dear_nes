// Copyright (c) 2020 Emmanuel Arias
#include "include/imgui_layer/imgui_nes_status_window.h"

#include <imgui.h>

void ImGuiNesStatusWindow::Render() {
    if (!m_Show) {
        return;
    }
    if (!Begin(m_WindowName)) {
        End();
        return;
    }
    bool buttonPressed = false;
    if (!m_IsPowerUp) {
        buttonPressed = ImGui::Button(m_PowerUpStr.c_str());
    } else {
        buttonPressed = ImGui::Button(m_ShutdownStr.c_str());
    }
    if (buttonPressed) {
        m_IsPowerUp = !m_IsPowerUp;
    }
    End();
}
