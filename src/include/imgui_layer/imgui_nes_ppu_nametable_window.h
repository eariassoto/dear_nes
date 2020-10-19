// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <string>

#include "include/imgui_layer/imgui_texture_image.h"
#include "include/imgui_layer/imgui_window.h"

class ImGuiNesPpuNametableWindow : public ImGuiWindow {
   public:
    ImGuiNesPpuNametableWindow(unsigned int nametableIdx);
    virtual void Render() override;

   private:
    unsigned int m_NametableIdx = 0;
    std::string m_WindowName;
};
