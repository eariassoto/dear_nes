// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <string>
#include "include/base_widget.h"

class CpuWidget : public BaseWidget {
   public:
    virtual void Render() override;

   private:
    const std::string m_WindowName = "CPU";
    int counter = 0;
};
