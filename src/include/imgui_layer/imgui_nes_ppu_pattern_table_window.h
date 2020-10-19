// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <string>

#include "include/imgui_layer/imgui_window.h"
#include "include/imgui_layer/imgui_texture_image.h"

class ImGuiNesPpuPatternTableWindow : public ImGuiWindow {
   public:
    ImGuiNesPpuPatternTableWindow(unsigned int patternTableIdx);
    virtual void Render() override;
    virtual void Update() override;

   private:
    void UpdatePatternTable();

    static constexpr unsigned int DEFAULT_PALETTE = 0;
    static constexpr unsigned int PATTERN_TABLE_REAL_WIDHT = 128;
    static constexpr unsigned int PATTERN_TABLE_REAL_HEIGHT = 128;

    unsigned int m_PatternTableIdx = 0;
    std::string m_WindowName;
    ImGuiTextureImage m_PpuPatternTable =
        ImGuiTextureImage{PATTERN_TABLE_REAL_WIDHT, PATTERN_TABLE_REAL_HEIGHT};
};
