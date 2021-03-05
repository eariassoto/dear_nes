// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <string>
#include <vector>
#include <chrono>

struct GLFWwindow;
class BaseWidget;

class ImGuiWidgetManager {
   public:
    ImGuiWidgetManager() = default;
    ~ImGuiWidgetManager();

    void Initialize(GLFWwindow* window);
    void Shutdown();

    void Update(std::chrono::nanoseconds delta);
    void Render();

   protected:
    BaseWidget* AddWidget(BaseWidget* newWin);

   private:
    std::vector<BaseWidget*> m_Widgets;

    void ShowDockSpace(bool* p_open);
    void SetStyle();
    void RenderWindows();
    void DeleteWindows();
};
