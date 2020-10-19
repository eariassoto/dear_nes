// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <string>
#include "include/imgui_layer/imgui_window.h"

class ImGuiNesCpuWindow : public ImGuiWindow {
   public:
    virtual void Render() override;

   private:
    const std::string m_WindowName = "CPU";
    int counter = 0;
};
