// Copyright (c) 2020-2021 Emmanuel Arias
#pragma once
#include <string>

#include "include/dearnes_base_widget.h"
#include "include/texture_image.h"

class PpuPatternTableWidget : public DearNESBaseWidget {
   public:
    PpuPatternTableWidget(dearnes::Nes* nesPtr, unsigned int patternTableIdx);
    
    virtual void Render() override;
    virtual void Update(float delta) override;

   private:
    void UpdatePatternTable();

    static constexpr unsigned int DEFAULT_PALETTE = 0;
    static constexpr unsigned int PATTERN_TABLE_REAL_WIDTH = 128;
    static constexpr unsigned int PATTERN_TABLE_REAL_HEIGHT = 128;

    unsigned int m_PatternTableIdx = 0;
    std::string m_WindowName;
    ImGuiTextureImage m_PpuPatternTable =
        ImGuiTextureImage{PATTERN_TABLE_REAL_WIDTH, PATTERN_TABLE_REAL_HEIGHT};
};
