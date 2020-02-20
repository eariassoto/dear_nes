// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <imgui.h>
#include <memory>

namespace cpuemulator {
class Cpu;

class CpuWidget {
   public:
    CpuWidget(const std::shared_ptr<Cpu> cpu);
    void Render();

   private:
    const std::shared_ptr<Cpu> m_Cpu;
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
