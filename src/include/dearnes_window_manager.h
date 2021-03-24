// Copyright (c) 2020-2021 Emmanuel Arias
#pragma once
#include <string>
#include "include/imgui_window_manager.h"

namespace dearnes {
class Nes;
}

class StatusWidget;

struct DearNESWindowManager : public ImGuiWindowManager {

    DearNESWindowManager(dearnes::Nes* nesPtr);

    virtual void RegisterWidgets() override;

    StatusWidget* m_NesStatusWindow = nullptr;

    private:
    virtual void ProcessInput(GLFWwindow* window) override;

    dearnes::Nes* m_NesPtr = nullptr;
};
