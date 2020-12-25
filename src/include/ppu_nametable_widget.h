// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <string>

#include "include/texture_image.h"
#include "include/base_widget.h"

class PpuNametableWidget : public BaseWidget {
   public:
    PpuNametableWidget(unsigned int nametableIdx);
    virtual void Render() override;

   private:
    unsigned int m_NametableIdx = 0;
    std::string m_WindowName;
};
