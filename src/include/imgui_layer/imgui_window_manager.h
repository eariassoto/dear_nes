// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <string>
#include <vector>

struct GLFWwindow;
class ImGuiWindow;

class ImGuiWindowManager {
   public:
    ImGuiWindowManager() = default;
    ~ImGuiWindowManager();

    void Initialize(GLFWwindow* window);
    void Shutdown();

    void Update();
    void Render();
    
    protected:
        ImGuiWindow* AddWindow(ImGuiWindow* newWin);

   private:
    std::vector<ImGuiWindow*> m_Windows;

    void ShowDockSpace(bool* p_open);
    void SetStyle();
    void RenderWindows();
    void DeleteWindows();
};
