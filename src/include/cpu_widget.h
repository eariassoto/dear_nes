// Copyright (c) 2020-2021 Emmanuel Arias
#pragma once
#include <string>
#include "include/dearnes_base_widget.h"

class CpuWidget : public DearNESBaseWidget {
   public:
   CpuWidget(dearnes::Nes* nesPtr);

    virtual void Update(float delta) override;
    virtual void Render() override;

   private:
    const std::string m_WindowName = "CPU";
    int counter = 0;
};
