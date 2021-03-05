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
    virtual void Update(std::chrono::nanoseconds delta) override;

   private:
    
    std::chrono::nanoseconds m_InitialRefreshValue =
        std ::chrono::duration_cast<std::chrono::nanoseconds>(1s);

    unsigned int m_NametableIdx = 0;
    std::array<uint8_t, 30 * 32> m_NametableValues;
    std::array<std::chrono::nanoseconds, 30 * 32> m_RefreshValues;
    std::string m_WindowName;
};
