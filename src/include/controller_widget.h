// Copyright (c) 2020-2021 Emmanuel Arias
#pragma once
#include <string>
#include "include/dearnes_base_widget.h"

class ControllerWidget : public DearNESBaseWidget {
   public:
    ControllerWidget(dearnes::Nes* nesPtr, unsigned int controllerIdx);

    virtual void Update(float delta) override;
    virtual void Render() override;

   private:
    unsigned int m_ControllerIdx = 0;
    std::string m_WindowName;

    static constexpr size_t NUM_BUTTONS = 8;
    static constexpr char* m_ButtonNames[NUM_BUTTONS] = {
        "Up", "Down", "Left", "Right",
                                   "A",  "B",    "Select", "Start"};
    static constexpr char* m_ButtonBinds[NUM_BUTTONS] = {
        "Arrow Up", "Arrow Down", "Arrow Left",
                                   "Arrow Right", "S",          "A",
                                   "Q",           "W"};
    static constexpr uint8_t m_ButtonMasks[NUM_BUTTONS] = {
        0x08, 0x04, 0x02, 0x01,
                                     0x80, 0x40, 0x20, 0x10};
};
