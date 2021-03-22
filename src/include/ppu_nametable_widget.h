// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <array>
#include <chrono>
#include <string>

#include "include/base_widget.h"
#include "include/texture_image.h"

using namespace std::chrono_literals;

class PpuNametableWidget : public BaseWidget {
   public:
    PpuNametableWidget(unsigned int nametableIdx);
    virtual void Render() override;
    virtual void Update(float delta) override;

   private:
    
    float m_InitialRefreshValue = 1.5f;

    unsigned int m_NametableIdx = 0;
    std::array<uint8_t, 30 * 32> m_NametableValues;
    std::array<float, 30 * 32> m_RefreshValues;
    std::string m_WindowName;
};
