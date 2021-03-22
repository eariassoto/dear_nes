// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <string>
#include "include/imgui_window_manager.h"

class StatusWidget;

struct DearNESWindowManager : public ImGuiWindowManager {

    DearNESWindowManager();

    virtual void RegisterWidgets() override;

    StatusWidget* m_NesStatusWindow = nullptr;

    private:
    virtual void ProcessInput(GLFWwindow* window) override;
};
