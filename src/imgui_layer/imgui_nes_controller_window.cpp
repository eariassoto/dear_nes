// Copyright (c) 2020 Emmanuel Arias
#include "include/imgui_layer/imgui_nes_controller_window.h"

#include <fmt/core.h>
#include <imgui.h>

#include "include/global_nes.h"
#include "include/nes.h"

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
    cpuemulator::Nes* nesEmulator = g_GetGlobalNes();
    ImGui::PushID(0);
    ImGui::Columns(3);
    uint8_t controllerByte = nesEmulator->m_Controllers[m_ControllerIdx];
    SetColorForButton((controllerByte & 0x08) != 0x00);
    ImGui::Button("Up");
    SetColorForButton((controllerByte & 0x02) != 0x00);
    ImGui::Button("Left");
    ImGui::SameLine();
    SetColorForButton((controllerByte & 0x01) != 0x00);
    ImGui::Button("Right");
    SetColorForButton((controllerByte & 0x04) != 0x00);
    ImGui::Button("Down");
    ImGui::NextColumn();
    SetColorForButton((controllerByte & 0x20) != 0x00);
    ImGui::Button("Select\n(Q)");
    ImGui::SameLine();
    SetColorForButton((controllerByte & 0x10) != 0x00);
    ImGui::Button("Start\n(W)");
    ImGui::NextColumn();
    SetColorForButton((controllerByte & 0x40) != 0x00);
    ImGui::Button("B\n(A)");
    ImGui::SameLine();
    SetColorForButton((controllerByte & 0x80) != 0x00);
    ImGui::Button("A\n(S)");
    SetColorForButton(false);
    ImGui::PopID();
    End();
}
