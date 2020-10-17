// Copyright (c) 2020 Emmanuel Arias
#include "include/imgui_layer/imgui_window.h"

#include <imgui.h>

void ImGuiWindow::Update() {}

void ImGuiWindow::Render() {}

bool ImGuiWindow::ShouldShow() const { return m_Show; }

bool ImGuiWindow::Begin(const std::string& name) {
    return ImGui::Begin(name.c_str(), &m_Show);
}

void ImGuiWindow::End() { ImGui::End(); }
