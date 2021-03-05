// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <string>

#include "include/base_widget.h"
#include "include/texture_image.h"

class PpuPalettesWidget : public BaseWidget {
   public:
    PpuPalettesWidget();
    virtual void Render() override;
    virtual void Update() override;

   private:
    void UpdatePatternTable();

    static constexpr unsigned int PALETTE_TABLE_REAL_WIDTH = 4;
    static constexpr unsigned int PALETTE_TABLE_REAL_HEIGHT = 4;

    std::string m_WindowName;
    ImGuiTextureImage m_PpuBackgroundPalette =
        ImGuiTextureImage{PALETTE_TABLE_REAL_WIDTH, PALETTE_TABLE_REAL_HEIGHT};
    ImGuiTextureImage m_PpuCharacterPalette =
        ImGuiTextureImage{PALETTE_TABLE_REAL_WIDTH, PALETTE_TABLE_REAL_HEIGHT};
};
