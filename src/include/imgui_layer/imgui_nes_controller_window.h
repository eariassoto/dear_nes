// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <string>
#include "include/imgui_layer/imgui_window.h"

class ImGuiNesControllerWindow : public ImGuiWindow {
   public:
    ImGuiNesControllerWindow(unsigned int controllerIdx);
    virtual void Render() override;

   private:
    unsigned int m_ControllerIdx = 0;
    std::string m_WindowName;
};
