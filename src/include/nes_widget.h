// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <memory>

namespace cpuemulator {
class Bus;

class NesWidget {
   public:
    NesWidget(std::shared_ptr<Bus>& nes);
    void Render();

	bool IsDoResetButtonClicked();
	bool IsDoFrameButtonClicked();
	bool IsDoStepButtonClicked();

	bool IsSimulationRunChecked();

   private:
    std::shared_ptr<Bus> m_Nes;

    bool m_ShouldSimulationRun = false;
    bool m_DoResetBtn = false;
    bool m_DoStepBtn = false;
    bool m_DoFrameBtn = false;
};

}  // namespace cpuemulator
