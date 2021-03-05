// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <string>

class BaseWidget {
   public:
    BaseWidget() = default;
    virtual ~BaseWidget() = default;

    virtual void Update();
    virtual void Render();
    // TODO: Public API Hide & show

   protected:
    bool Begin(const std::string& name);
    void End();
    bool m_Show = true;
};
