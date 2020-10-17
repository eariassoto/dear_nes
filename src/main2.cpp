// Copyright (c) 2020 Emmanuel Arias
// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
// clang-format on
#include <iostream>

#include "include/imgui_layer/imgui_sample_window.h"
#include "include/imgui_layer/imgui_window_manager.h"
#include "include/imgui_layer/imgui_nes_screen_window.h"
#include "include/imgui_layer/imgui_nes_status_window.h"

// Forward declaration
void framebufferSizeCallback2(GLFWwindow* window, int width, int height);
void processInput2(GLFWwindow* window);

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // UiConfig uiConfig;
    int screenWidth = 1250;
    int screenHeight = 800;
    GLFWwindow* window =
        glfwCreateWindow(screenWidth, screenHeight, "NES Emulator", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << '\n';
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << '\n';
        return -1;
    }

    glViewport(0, 0, screenWidth, screenHeight);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback2);

    ImGuiWindowManager uiManager;
    uiManager.Initialize(window);
    uiManager.AddWindow(new ImGuiNesScreenWindow());
    ImGuiNesStatusWindow* nesStatusWindow = dynamic_cast<ImGuiNesStatusWindow*>(
        uiManager.AddWindow(new ImGuiNesStatusWindow()));

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        processInput2(window);

        uiManager.Update();
        uiManager.Render();

        glfwSwapBuffers(window);
    }

    // logger.Stop();
    // delete nesEmulator;

    // Cleanup
    uiManager.Shutdown();
    glfwTerminate();
}

void framebufferSizeCallback2(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput2(GLFWwindow* window) {}