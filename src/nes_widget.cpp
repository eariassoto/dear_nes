// Copyright (c) 2020 Emmanuel Arias
#include <imgui.h>

#include "include/nes_widget.h"
#include "include/bus.h"

namespace cpuemulator {

NesWidget::NesWidget(std::shared_ptr<Bus>& nes) : m_Nes{nes} {}

void NesWidget::Render() {
    ImGui::Begin("NES controls");
    ImGui::SetWindowSize({230, 100});
    ImGui::Text("System Clock: %d", m_Nes->GetSystemClockCounter());
    ImGui::Checkbox("Run Simulation", &m_ShouldSimulationRun);

    if (!m_ShouldSimulationRun) {
        m_DoStepBtn = ImGui::Button("Step by Step");
        ImGui::SameLine();
        m_DoFrameBtn = ImGui::Button("Frame");
        ImGui::SameLine();
        m_DoResetBtn = ImGui::Button("Reset");
    }
    ImGui::End();
}

bool NesWidget::IsDoResetButtonClicked() {
    if (m_DoResetBtn == true) {
        m_DoResetBtn = false;
        return true;
    }
    return false;
}

bool NesWidget::IsDoFrameButtonClicked() {
    if (m_DoFrameBtn == true) {
        m_DoFrameBtn = false;
        return true;
    }
    return false;
}

bool NesWidget::IsDoStepButtonClicked() {
    if (m_DoStepBtn == true) {
        m_DoStepBtn = false;
        return true;
    }
    return false;
}

bool NesWidget::IsSimulationRunChecked() { return m_ShouldSimulationRun; }

}  // namespace cpuemulator
