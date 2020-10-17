// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <string>
#include "include/imgui_layer/imgui_window.h"

class IGStartupWindow : public ImGuiWindow {
   public:
    virtual void Update() override;

   private:
    const std::string m_WindowName = "Startup";
    int counter = 0;
};
