// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <string>
#include "include/imgui_layer/imgui_window.h"

class ImGuiNesStatusWindow : public ImGuiWindow {
   public:
    virtual void Render() override;
    bool IsNesPoweredUp() const;

   private:
    const std::string m_WindowName = "NES Console Status";
    const std::string m_PowerUpStr = "Power Up";
    const std::string m_ShutdownStr = "Shutdown";
    bool m_IsPowerUp = false;
};
