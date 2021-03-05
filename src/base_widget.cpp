// Copyright (c) 2020 Emmanuel Arias
#include "include/base_widget.h"

#include <imgui.h>

void BaseWidget::Update() {}

void BaseWidget::Render() {}

bool BaseWidget::Begin(const std::string& name) {
    return ImGui::Begin(name.c_str(), &m_Show);
}

void BaseWidget::End() { ImGui::End(); }
