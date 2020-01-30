#include "include/cpu_widget.h"
#include "include/cpu.h"

namespace cpuemulator {

CpuWidget::CpuWidget(Cpu& cpu)
    : m_Cpu{cpu},
      m_ColorFlagSet{.0f, 1.f, .0f, 1.f},
      m_ColorFlagReset{1.f, .0f, .0f, 1.f} {}

void CpuWidget::Render() {
    ImGui::Begin("CPU registers");
    ImGui::SetWindowSize({200, 140});

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

    ImGui::Text("PC: $%04x", m_Cpu.GetProgramCounter());
    uint8_t regA = m_Cpu.GetRegisterA();
    ImGui::Text("A: $%02x [%d]", regA, regA);
    uint8_t regX = m_Cpu.GetRegisterX();
    ImGui::Text("X: $%02x [%d]", regX, regX);
    uint8_t regY = m_Cpu.GetRegisterY();
    ImGui::Text("Y: $%02x [%d]", regY, regY);
    ImGui::Text("Stack Pointer: $%04x", m_Cpu.GetStackPointer());
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
