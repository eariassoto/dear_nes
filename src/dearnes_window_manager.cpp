// Copyright (c) 2020-2021 Emmanuel Arias
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

#include <GLFW/glfw3.h>

using Nes = dearnes::Nes;
DearNESWindowManager::DearNESWindowManager(dearnes::Nes* nesPtr)
: m_NesPtr{nesPtr} {}

void DearNESWindowManager::RegisterWidgets() {
    AddWidget(new ScreenWidget(m_NesPtr));
    m_NesStatusWindow =
        dynamic_cast<StatusWidget*>(AddWidget(new StatusWidget(m_NesPtr)));
    AddWidget(new CpuWidget(m_NesPtr));
    AddWidget(new PpuPalettesWidget(m_NesPtr));
    AddWidget(new PpuPatternTableWidget(m_NesPtr, 0));
    AddWidget(new PpuPatternTableWidget(m_NesPtr, 1));
    AddWidget(new ControllerWidget(m_NesPtr, 0));
    AddWidget(new PpuNametableWidget(m_NesPtr, 0));
    AddWidget(new PpuNametableWidget(m_NesPtr, 1));
}

void DearNESWindowManager::ProcessInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    // TODO: define constants for NES buttons
    using dearnes::CONTROLLER_PLAYER_1_IDX;
    m_NesPtr->ClearControllerState(CONTROLLER_PLAYER_1_IDX);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        m_NesPtr->WriteControllerState(CONTROLLER_PLAYER_1_IDX, 0x80);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        m_NesPtr->WriteControllerState(CONTROLLER_PLAYER_1_IDX, 0x40);
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        m_NesPtr->WriteControllerState(CONTROLLER_PLAYER_1_IDX, 0x20);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        m_NesPtr->WriteControllerState(CONTROLLER_PLAYER_1_IDX, 0x10);
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        m_NesPtr->WriteControllerState(CONTROLLER_PLAYER_1_IDX, 0x08);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        m_NesPtr->WriteControllerState(CONTROLLER_PLAYER_1_IDX, 0x04);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        m_NesPtr->WriteControllerState(CONTROLLER_PLAYER_1_IDX, 0x02);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        m_NesPtr->WriteControllerState(CONTROLLER_PLAYER_1_IDX, 0x01);
    }
}
