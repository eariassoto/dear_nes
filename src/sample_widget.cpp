// Copyright (c) 2020 Emmanuel Arias

#include "include/sample_widget.h"

#include <imgui.h>

void SampleWidget::Render() {
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
