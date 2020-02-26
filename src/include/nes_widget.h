// Copyright (c) 2020 Emmanuel Arias
#pragma once

namespace cpuemulator {
class Nes;

class NesWidget {
   public:
    NesWidget(Nes* nes);
    void Render();

    bool IsDoResetButtonClicked();
    bool IsDoFrameButtonClicked();
    bool IsDoStepButtonClicked();

    bool IsSimulationRunChecked();

   private:
    Nes* m_Nes = nullptr;

    bool m_ShouldSimulationRun = false;
    bool m_DoResetBtn = false;
    bool m_DoStepBtn = false;
    bool m_DoFrameBtn = false;
};

}  // namespace cpuemulator
