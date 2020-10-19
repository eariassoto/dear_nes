// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <string>

#include "include/imgui_layer/imgui_texture_image.h"
#include "include/imgui_layer/imgui_window.h"

class ImGuiNesScreenWindow : public ImGuiWindow {
   public:
    virtual void Update() override;
    virtual void Render() override;

   private:
    static constexpr unsigned int SCREEN_REAL_WIDHT = 256;
    static constexpr unsigned int SCREEN_REAL_HEIGHT = 240;
    const std::string m_WindowName = "NES Screen";
    ImGuiTextureImage m_NesScreenWidget = ImGuiTextureImage{
        SCREEN_REAL_WIDHT, SCREEN_REAL_HEIGHT};
};
