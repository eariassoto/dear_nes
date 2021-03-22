// Copyright (c) 2020 Emmanuel Arias
#include "include/dearnes_window_manager.h"

#include <imgui.h>

#include "dear_nes_lib/nes.h"
#include "include/controller_widget.h"
#include "include/cpu_widget.h"
#include "include/ppu_nametable_widget.h"
#include "include/ppu_palettes_widget.h"
#include "include/ppu_pattern_table_widget.h"
#include "include/screen_widget.h"
#include "include/status_widget.h"
#include "include/global_nes.h"

#include <GLFW/glfw3.h>

using Nes = dearnes::Nes;
DearNESWindowManager::DearNESWindowManager() {
    
}

void DearNESWindowManager::RegisterWidgets() {
    AddWidget(new ScreenWidget());
    m_NesStatusWindow =
        dynamic_cast<StatusWidget*>(AddWidget(new StatusWidget()));
    AddWidget(new CpuWidget());
    AddWidget(new PpuPalettesWidget());
    AddWidget(new PpuPatternTableWidget(0));
    AddWidget(new PpuPatternTableWidget(1));
    AddWidget(new ControllerWidget(0));
    AddWidget(new PpuNametableWidget(0));
    AddWidget(new PpuNametableWidget(1));
}

void DearNESWindowManager::ProcessInput(GLFWwindow* window) {
    Nes* nesEmulator = g_GetGlobalNes();
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    // TODO: define constants for NES buttons
    using dearnes::CONTROLLER_PLAYER_1_IDX;
    nesEmulator->ClearControllerState(CONTROLLER_PLAYER_1_IDX);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        nesEmulator->WriteControllerState(CONTROLLER_PLAYER_1_IDX, 0x80);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        nesEmulator->WriteControllerState(CONTROLLER_PLAYER_1_IDX, 0x40);
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        nesEmulator->WriteControllerState(CONTROLLER_PLAYER_1_IDX, 0x20);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        nesEmulator->WriteControllerState(CONTROLLER_PLAYER_1_IDX, 0x10);
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        nesEmulator->WriteControllerState(CONTROLLER_PLAYER_1_IDX, 0x08);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        nesEmulator->WriteControllerState(CONTROLLER_PLAYER_1_IDX, 0x04);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        nesEmulator->WriteControllerState(CONTROLLER_PLAYER_1_IDX, 0x02);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        nesEmulator->WriteControllerState(CONTROLLER_PLAYER_1_IDX, 0x01);
    }
}
