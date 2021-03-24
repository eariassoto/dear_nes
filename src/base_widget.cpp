// Copyright (c) 2020-2021 Emmanuel Arias
#include "include/base_widget.h"

#include <imgui.h>

bool BaseWidget::Begin(const std::string& name) {
    return ImGui::Begin(name.c_str(), &m_Show);
}

void BaseWidget::End() { ImGui::End(); }
