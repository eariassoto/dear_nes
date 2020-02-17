// Copyright (c) 2020 Emmanuel Arias
#include "include/nes_widget.h"
#include "include/bus.h"

namespace cpuemulator {

NesWidget::NesWidget(Bus& bus) : m_Bus{bus} {}

void NesWidget::Render() {
    ImGui::Begin("NES controls");
    ImGui::SetWindowSize({230, 100});
    ImGui::Text("System Clock: %d", m_Bus.m_SystemClockCounter);
    ImGui::Checkbox("Run Simulation", &m_ShouldSimulationRun);
    if (!m_ShouldSimulationRun) {
        bool step = ImGui::Button("Step by Step");
        if (step) {
            do {
                m_Bus.Clock();
            } while (!m_Bus.m_Cpu.InstructionComplete());

            do {
                m_Bus.Clock();
            } while (m_Bus.m_Cpu.InstructionComplete());
        }
        ImGui::SameLine();
        bool reset = ImGui::Button("Reset");
        if (reset) {
            m_Bus.Reset();
        }
        ImGui::SameLine();
        bool frame = ImGui::Button("Frame");
        if (frame) {
            DoNesFrame();
        }
    } else {
        DoNesFrame();
    }
    ImGui::End();
}

void NesWidget::DoNesFrame() {
    do {
        m_Bus.Clock();
    } while (!m_Bus.m_Ppu.isFrameComplete);

    do {
        m_Bus.m_Cpu.Clock();
    } while (m_Bus.m_Cpu.InstructionComplete());

    m_Bus.m_Ppu.isFrameComplete = false;
}

}  // namespace cpuemulator
