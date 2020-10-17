// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <string>

struct GLFWwindow;

class ImGuiWindow {
   public:
    ImGuiWindow();
    virtual ~ImGuiWindow();
    virtual void Update();
    void Show();

   protected:
    virtual bool Begin(const std::string& name);
    void End();
    bool m_Show = true;
};
