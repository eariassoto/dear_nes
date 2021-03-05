// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <string>
#include <chrono>

class BaseWidget {
   public:
    BaseWidget() = default;
    virtual ~BaseWidget() = default;

    virtual void Update(std::chrono::nanoseconds delta);
    virtual void Render();
    // TODO: Public API Hide & show

   protected:
    bool Begin(const std::string& name);
    void End();
    bool m_Show = true;
};
