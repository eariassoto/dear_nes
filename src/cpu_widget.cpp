// Copyright (c) 2020 Emmanuel Arias
#include "include/cpu_widget.h"
#include "include/cpu.h"

namespace cpuemulator {

CpuWidget::CpuWidget(Cpu& cpu)
    : m_Cpu{cpu},
      m_ColorFlagSet{.0f, 1.f, .0f, 1.f},
      m_ColorFlagReset{1.f, .0f, .0f, 1.f} {}

void CpuWidget::Render() {
    ImGui::Begin("CPU registers");
    ImGui::SetWindowSize({280, 160});

    ImGui::Text("Status: ");
    ImGui::SameLine();
    ImGui::TextColored(GetColorForFlag(m_Cpu.GetFlag(Cpu::FLAGS::N)), "N");
    ImGui::SameLine();
    ImGui::TextColored(GetColorForFlag(m_Cpu.GetFlag(Cpu::FLAGS::V)), "V");
    ImGui::SameLine();
    ImGui::TextColored(GetColorForFlag(false), "-");
    ImGui::SameLine();
    ImGui::TextColored(GetColorForFlag(m_Cpu.GetFlag(Cpu::FLAGS::B)), "B");
    ImGui::SameLine();
    ImGui::TextColored(GetColorForFlag(m_Cpu.GetFlag(Cpu::FLAGS::D)), "D");
    ImGui::SameLine();
    ImGui::TextColored(GetColorForFlag(m_Cpu.GetFlag(Cpu::FLAGS::I)), "I");
    ImGui::SameLine();
    ImGui::TextColored(GetColorForFlag(m_Cpu.GetFlag(Cpu::FLAGS::Z)), "Z");
    ImGui::SameLine();
    ImGui::TextColored(GetColorForFlag(m_Cpu.GetFlag(Cpu::FLAGS::C)), "C");

    uint8_t regA = m_Cpu.m_RegisterA;
    ImGui::Text("A: $%02X [%d]", regA, regA);
    uint8_t regX = m_Cpu.m_RegisterX;
    ImGui::Text("X: $%02X [%d]", regX, regX);
    uint8_t regY = m_Cpu.m_RegisterY;
    ImGui::Text("Y: $%02X [%d]", regY, regY);
    ImGui::Text("Stack Pointer: $%04X", m_Cpu.m_StackPointer);
    ImGui::Text("PC: $%04X", m_Cpu.m_ProgramCounter);
    ImGui::Text("Instruction at PC: %s", m_Cpu.GetInstructionString(m_Cpu.m_ProgramCounter).c_str());
    ImGui::End();

    ImGui::Begin("CPU controls");
    ImGui::SetWindowSize({200, 60});
    ImGui::SameLine();
    bool step = ImGui::Button("Step");
    if (step) {
        do {
            m_Cpu.Clock();
        } while (!m_Cpu.InstructionComplete());
    }
    ImGui::SameLine();
    bool reset = ImGui::Button("Reset");
	if (reset)
	{
        m_Cpu.Reset();
	}
    ImGui::SameLine();
    bool irq = ImGui::Button("IRQ");
	if (irq)
	{
        m_Cpu.InterruptRequest();
	}
    ImGui::SameLine();
    bool nmi = ImGui::Button("NMI");
    if (nmi) {
        m_Cpu.NonMaskableInterrupt();
    }
    ImGui::End();
}

}  // namespace cpuemulator
