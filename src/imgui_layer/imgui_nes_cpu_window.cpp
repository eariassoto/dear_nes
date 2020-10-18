// Copyright (c) 2020 Emmanuel Arias
#include "include/imgui_layer/imgui_nes_cpu_window.h"

#include <imgui.h>

#include "include/global_nes.h"
#include "include/nes.h"

void ImGuiNesCpuWindow::Render() {
    if (!m_Show) {
        return;
    }
    if (!Begin(m_WindowName)) {
        End();
        return;
    }
    auto GetColorForFlag = [](bool status) -> const ImVec4& {
        static ImVec4 m_ColorFlagSet{.0f, 1.f, .0f, 1.f};
        static ImVec4 m_ColorFlagReset{1.f, .0f, .0f, 1.f};
        if (status) {
            return m_ColorFlagSet;
        } else {
            return m_ColorFlagReset;
        }
    };
    cpuemulator::Nes* nesEmulator = g_GetGlobalNes();
    auto cpuPtr = nesEmulator->m_Virtual6502;

    ImGui::Text("Status: ");
    ImGui::SameLine();
    ImGui::TextColored(GetColorForFlag(cpuPtr->GetFlag(Virtual6502Flag::N)),
                       "N");
    ImGui::SameLine();
    ImGui::TextColored(GetColorForFlag(cpuPtr->GetFlag(Virtual6502Flag::V)),
                       "V");
    ImGui::SameLine();
    ImGui::TextColored(GetColorForFlag(false), "-");
    ImGui::SameLine();
    ImGui::TextColored(GetColorForFlag(cpuPtr->GetFlag(Virtual6502Flag::B)),
                       "B");
    ImGui::SameLine();
    ImGui::TextColored(GetColorForFlag(cpuPtr->GetFlag(Virtual6502Flag::D)),
                       "D");
    ImGui::SameLine();
    ImGui::TextColored(GetColorForFlag(cpuPtr->GetFlag(Virtual6502Flag::I)),
                       "I");
    ImGui::SameLine();
    ImGui::TextColored(GetColorForFlag(cpuPtr->GetFlag(Virtual6502Flag::Z)),
                       "Z");
    ImGui::SameLine();
    ImGui::TextColored(GetColorForFlag(cpuPtr->GetFlag(Virtual6502Flag::C)),
                       "C");

    uint8_t regA = cpuPtr->GetRegisterA();
    ImGui::Text("A: $%02X [%d]", regA, regA);
    uint8_t regX = cpuPtr->GetRegisterX();
    ImGui::Text("X: $%02X [%d]", regX, regX);
    uint8_t regY = cpuPtr->GetRegisterY();
    ImGui::Text("Y: $%02X [%d]", regY, regY);
    ImGui::Text("Stack Pointer: $%04X", cpuPtr->GetStackPointer());
    ImGui::Text("PC: $%04X", cpuPtr->GetProgramCounter());
    // ImGui::Text("Instruction at PC: %s",
    //            cpuPtr->GetInstructionString(m_Virtual6502->m_ProgramCounter).c_str());
    End();
}