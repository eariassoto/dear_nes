// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <string>
#include <vector>

struct GLFWwindow;
class BaseWidget;

class ImGuiWidgetManager {
   public:
    ImGuiWidgetManager() = default;
    ~ImGuiWidgetManager();

    void Initialize(GLFWwindow* window);
    void Shutdown();

    void Update();
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
