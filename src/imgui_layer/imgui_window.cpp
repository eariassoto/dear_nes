// Copyright (c) 2020 Emmanuel Arias
#include "include/imgui_layer/imgui_window.h"

#include <imgui.h>

ImGuiWindow::ImGuiWindow() {}

ImGuiWindow::~ImGuiWindow() {}

void ImGuiWindow::Update() {}

void ImGuiWindow::Show() {}

bool ImGuiWindow::Begin(const std::string& name) {
    return ImGui::Begin(name.c_str(), &m_Show);
}

void ImGuiWindow::End() { ImGui::End(); }
