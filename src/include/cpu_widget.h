// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <imgui.h>
#include "include/virtual6502.h"

namespace cpuemulator {

template <class MemoryInterfaceT>
class CpuWidget {
   public:
    CpuWidget(Virtual6502<MemoryInterfaceT>* cpu)
        : m_Cpu{cpu},
          m_ColorFlagSet{.0f, 1.f, .0f, 1.f},
          m_ColorFlagReset{1.f, .0f, .0f, 1.f} {}

    void Render() { /*
					ImGui::Begin("CPU registers");
        ImGui::SetWindowSize({280, 160});

        ImGui::Text("Status: ");
        ImGui::SameLine();
        ImGui::TextColored(
            GetColorForFlag(m_Cpu->GetFlag(Virtual6502::Flag::N)), "N");
        ImGui::SameLine();
        ImGui::TextColored(
            GetColorForFlag(m_Cpu->GetFlag(Virtual6502::Flag::V)), "V");
        ImGui::SameLine();
        ImGui::TextColored(GetColorForFlag(false), "-");
        ImGui::SameLine();
        ImGui::TextColored(
            GetColorForFlag(m_Cpu->GetFlag(Virtual6502::Flag::B)), "B");
        ImGui::SameLine();
        ImGui::TextColored(
            GetColorForFlag(m_Cpu->GetFlag(Virtual6502::Flag::D)), "D");
        ImGui::SameLine();
        ImGui::TextColored(
            GetColorForFlag(m_Cpu->GetFlag(Virtual6502::Flag::I)), "I");
        ImGui::SameLine();
        ImGui::TextColored(
            GetColorForFlag(m_Cpu->GetFlag(Virtual6502::Flag::Z)), "Z");
        ImGui::SameLine();
        ImGui::TextColored(
            GetColorForFlag(m_Cpu->GetFlag(Virtual6502::Flag::C)), "C");

        uint8_t regA = m_Cpu->GetRegisterA();
        ImGui::Text("A: $%02X [%d]", regA, regA);
        uint8_t regX = m_Cpu->GetRegisterX();
        ImGui::Text("X: $%02X [%d]", regX, regX);
        uint8_t regY = m_Cpu->GetRegisterY();
        ImGui::Text("Y: $%02X [%d]", regY, regY);
        ImGui::Text("Stack Pointer: $%04X", m_Cpu->GetStackPointer());
        ImGui::Text("PC: $%04X", m_Cpu->GetProgramCounter());
        // ImGui::Text("Instruction at PC: %s",
        //            m_Cpu->GetInstructionString(m_Cpu->m_ProgramCounter).c_str());
        ImGui::End();*/
    }

   private:
    Virtual6502<MemoryInterfaceT>* m_Cpu = nullptr;
    ImVec4 m_ColorFlagSet;
    ImVec4 m_ColorFlagReset;

    inline const ImVec4& GetColorForFlag(bool status) const {
        if (status) {
            return m_ColorFlagSet;
        } else {
            return m_ColorFlagReset;
        }
    }
};

}  // namespace cpuemulator
