// Copyright (c) 2020 Emmanuel Arias
#include "include/imgui_layer/imgui_nes_controller_window.h"

#include <fmt/core.h>
#include <imgui.h>

#include <cinttypes>

#include "include/global_nes.h"
#include "virtual-nes/nes.h"

ImGuiNesControllerWindow::ImGuiNesControllerWindow(unsigned int controllerIdx)
    : m_ControllerIdx{controllerIdx},
      m_WindowName{fmt::format("Controller {}", (m_ControllerIdx + 1))} {}

void ImGuiNesControllerWindow::Render() {
    if (!m_Show) {
        return;
    }
    if (!Begin(m_WindowName)) {
        End();
        return;
    }
    auto SetColorForButton = [](bool isPressed) {
        static bool isStyledPushed = false;
        if (isPressed) {
            if (!isStyledPushed) {
                ImGui::PushStyleColor(ImGuiCol_Button,
                                      (ImVec4)ImColor::HSV(7.0f, 0.6f, 0.6f));
                isStyledPushed = true;
            }
        } else {
            if (isStyledPushed) {
                ImGui::PopStyleColor();
                isStyledPushed = false;
            }
        }
    };
    virtualnes::Nes* nesEmulator = g_GetGlobalNes();
    ImGui::Columns(3, "mycolumns");  // 4-ways, with border
    ImGui::Separator();
    ImGui::Text("Button");
    ImGui::NextColumn();
    ImGui::Text("Keybind");
    ImGui::NextColumn();
    ImGui::Text("Is Pressed");
    ImGui::NextColumn();
    ImGui::Separator();

    static int selected = -1;
    bool test = true;
    uint8_t controllerByte = nesEmulator->GetControllerState(m_ControllerIdx);
    for (int i = 0; i < NUM_BUTTONS; i++) {
        ImGui::Text(m_ButtonNames[i]);
        ImGui::NextColumn();
        ImGui::Text(m_ButtonBinds[i]);
        ImGui::NextColumn();
        test = (controllerByte & m_ButtonMasks[i]);
        ImGui::Checkbox("", &test);
        ImGui::NextColumn();
    }
    ImGui::Columns(1);
    ImGui::Separator();
    //ImGui::TreePop();
    End();
}
