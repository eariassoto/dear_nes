// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <string>

struct GLFWwindow;

class BaseWidget {
   public:
    BaseWidget() = default;
    virtual ~BaseWidget() = default;

    virtual void Update();
    virtual void Render();
    bool ShouldShow() const;

   protected:
    bool Begin(const std::string& name);
    void End();
    bool m_Show = true;
};
