// Copyright (c) 2020 Emmanuel Arias
#include "include/imgui_layer/imgui_nes_window_manager.h"

#include <imgui.h>

#include "include/imgui_layer/imgui_nes_controller_window.h"
#include "include/imgui_layer/imgui_nes_cpu_window.h"
#include "include/imgui_layer/imgui_nes_ppu_nametable_window.h"
#include "include/imgui_layer/imgui_nes_ppu_pattern_table_window.h"
#include "include/imgui_layer/imgui_nes_screen_window.h"
#include "include/imgui_layer/imgui_nes_status_window.h"

ImGuiNesWindowManager::ImGuiNesWindowManager() {
    AddWindow(new ImGuiNesScreenWindow());
    m_NesStatusWindow = dynamic_cast<ImGuiNesStatusWindow*>(
        AddWindow(new ImGuiNesStatusWindow()));
    AddWindow(new ImGuiNesCpuWindow());
    AddWindow(new ImGuiNesPpuPatternTableWindow(0));
    AddWindow(new ImGuiNesPpuPatternTableWindow(1));
    AddWindow(new ImGuiNesControllerWindow(0));
    // AddWindow(new ImGuiNesPpuNametableWindow(0));
    // AddWindow(new ImGuiNesPpuNametableWindow(1));
}
