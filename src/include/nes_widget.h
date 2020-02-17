// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <imgui.h>

namespace cpuemulator {
class Bus;

class NesWidget {
   public:
    NesWidget(Bus& cpu);
    void Render();

   private:
    Bus& m_Bus;

    bool m_ShouldSimulationRun = false;
    void DoNesFrame();
};

}  // namespace cpuemulator
