// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <imgui.h>

namespace cpuemulator {
class Cpu;

class CpuWidget {
   public:
    CpuWidget(Cpu* cpu);
    void Render();

   private:
    Cpu* m_Cpu = nullptr;
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
