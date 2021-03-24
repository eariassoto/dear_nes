// Copyright (c) 2020-2021 Emmanuel Arias
#pragma once
#include <string>

#include "include/dearnes_base_widget.h"
#include "dear_nes_lib/cartridge_loader.h"

class StatusWidget : public DearNESBaseWidget {
   public:
   StatusWidget(dearnes::Nes* nesPtr);

   virtual void Update(float delta) override;
    virtual void Render() override;
    bool IsNesPoweredUp() const;

   private:
    const std::string m_WindowName = "NES Console Status";
    const std::string m_PowerUpStr = "Power Up";
    const std::string m_ShutdownStr = "Shutdown";
    bool m_IsPowerUp = false;

#ifdef _WIN32
    dearnes::CartridgeLoader m_CartridgeLoader;
    std::wstring GetFileFromUser();
#endif
};
