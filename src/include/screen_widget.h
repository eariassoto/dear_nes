// Copyright (c) 2020-2021 Emmanuel Arias
#pragma once
#include <string>

#include "include/dearnes_base_widget.h"
#include "include/base_widget.h"
#include "include/texture_image.h"

class ScreenWidget : public DearNESBaseWidget {
   public:
ScreenWidget(dearnes::Nes* nesPtr);

    virtual void Update(float delta) override;
    virtual void Render() override;

   private:
    static constexpr unsigned int SCREEN_REAL_WIDHT = 256;
    static constexpr unsigned int SCREEN_REAL_HEIGHT = 240;
    const std::string m_WindowName = "NES Screen";
    ImGuiTextureImage m_NesScreenTextureImage = ImGuiTextureImage{
        SCREEN_REAL_WIDHT, SCREEN_REAL_HEIGHT};
};
