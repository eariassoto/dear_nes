// Copyright (c) 2020 Emmanuel Arias
#include "include/imgui_layer/imgui_nes_screen_window.h"

#include <imgui.h>

void ImGuiNesScreenWindow::Render() {
    if (m_Show) {
        if (!Begin(m_WindowName)) {
            End();
        } else {
            m_NesScreenWidget.Render();
            End();
        }
    }
}

void ImGuiNesScreenWindow::Update() {
    if (m_Show) {
        m_NesScreenWidget.Update();
    }
}
