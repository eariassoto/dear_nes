// Copyright (c) 2020-2021 Emmanuel Arias
#pragma once
#include <vector>

class GLFWwindow;
class BaseWidget;

class ImGuiWindowManager {
   public:
    ImGuiWindowManager();
    ~ImGuiWindowManager();

    bool InitializeWindow();
    bool ShouldClose();
    void HandleEvents();

    virtual void RegisterWidgets() = 0;
    virtual void ProcessInput(GLFWwindow* window) = 0;

    void Update(float deltaTime);
    void Render();

    void DestroyWindow();

   protected:
    BaseWidget* AddWidget(BaseWidget* newWidget);

   private:
    GLFWwindow* m_Window = nullptr;
    std::vector<BaseWidget*> m_Widgets;

    void ShowDockSpace(bool* p_open);
};
