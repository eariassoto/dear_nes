// Copyright (c) 2021 Emmanuel Arias
#pragma once
#include "include/base_widget.h"

namespace dearnes {
class Nes;
}

class DearNESBaseWidget : public BaseWidget {
   public:
    DearNESBaseWidget(dearnes::Nes* nesPtr);
    virtual ~DearNESBaseWidget() = default;

    virtual void Update(float delta) = 0;
    virtual void Render() = 0;

   protected:
    dearnes::Nes* m_NesPtr = nullptr;
};
