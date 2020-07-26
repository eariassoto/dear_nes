// Copyright (c) 2020 Emmanuel Arias
#pragma once

namespace cpuemulator {

struct UiConfig {
    bool m_EmulatorIsRunning = false;
    bool m_EmulatorMustReset = false;

	bool m_PpuShowPatternTable0 = true;
	bool m_PpuShowPatternTable1 = true;

	void ResetEvents();
};

}  // namespace cpuemulator
