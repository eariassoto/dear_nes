// Copyright (c) 2020 Emmanuel Arias
#include "include/imgui_layer/imgui_sample_window.h"

#include <imgui.h>

void IGStartupWindow::Render() {
    if (m_Show) {
        if (!Begin(m_WindowName)) {
            End();
        } else {
            ImGui::Text("Press me:");
            if (ImGui::Button("button")) {
                // do something
            }
            End();
        }
    }
}
