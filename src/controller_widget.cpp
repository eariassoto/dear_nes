// Copyright (c) 2020-2021 Emmanuel Arias
#include "include/controller_widget.h"

#include <fmt/core.h>
#include <imgui.h>

#include <cinttypes>

#include "include/dearnes_base_widget.h"
#include "dear_nes_lib/nes.h"

ControllerWidget::ControllerWidget(dearnes::Nes* nesPtr, unsigned int controllerIdx)
    : DearNESBaseWidget(nesPtr)
    , m_ControllerIdx{controllerIdx}
    , m_WindowName{fmt::format("Controller {}", (m_ControllerIdx + 1))} {}

void ControllerWidget::Update(float delta) {}

void ControllerWidget::Render() {
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
    uint8_t controllerByte = m_NesPtr->GetControllerState(m_ControllerIdx);
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
