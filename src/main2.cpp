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
#include "include/imgui_layer/imgui_nes_cpu_window.h"
#include "include/nes.h"
#include "include/cartridge.h"
#include "include/global_nes.h"
//#include "include/file_manager.h"
//#include "include/sprite.h"
#include "include/logger.h"
#include "include/ui_config.h"

using Nes = cpuemulator::Nes;
using Cartridge = cpuemulator::Cartridge;
using Logger = cpuemulator::Logger;
using UiConfig = cpuemulator::UiConfig;

// Forward declaration
void framebufferSizeCallback2(GLFWwindow* window, int width, int height);
void processInput2(GLFWwindow* window);

int main(int argc, char* argv[]) {
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

    UiConfig uiConfig;
    Nes* nesEmulator = g_GetGlobalNes();

    if (argc > 1) {
        Cartridge* cartridge = new Cartridge(argv[1]);
        if (cartridge->IsLoaded()) {
            nesEmulator->InsertCatridge(cartridge);
        }
    }

    nesEmulator->Reset();

    Logger& logger = Logger::Get();
    logger.Start();

    ImGuiWindowManager uiManager;
    uiManager.Initialize(window);
    uiManager.AddWindow(new ImGuiNesScreenWindow());
    ImGuiNesStatusWindow* nesStatusWindow = dynamic_cast<ImGuiNesStatusWindow*>(
        uiManager.AddWindow(new ImGuiNesStatusWindow()));
    uiManager.AddWindow(new ImGuiNesCpuWindow());

    using clock = std::chrono::high_resolution_clock;
    using namespace std::chrono;
    static constexpr nanoseconds frameTime{1000ms / 60};

    nanoseconds residualTime = 0ms;
    auto frameStartTime = clock::now();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        processInput2(window);

        auto deltaTime = clock::now() - frameStartTime;
        frameStartTime = clock::now();

        if (nesStatusWindow->IsNesPoweredUp()) {
            if (residualTime > 0ns) {
                residualTime -= deltaTime;
            } else {
                residualTime += frameTime - deltaTime;

                nesEmulator->DoFrame();
            }
        }
        nesEmulator->Render();

        uiManager.Update();
        uiManager.Render();

        glfwSwapBuffers(window);
    }

    logger.Stop();
    // delete nesEmulator;

    // Cleanup
    uiManager.Shutdown();
    glfwTerminate();
}

void framebufferSizeCallback2(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput2(GLFWwindow* window) {
    Nes* nesEmulator = g_GetGlobalNes();
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    nesEmulator->m_Controllers[0] = 0x00;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        nesEmulator->m_Controllers[0] |= 0x80;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        nesEmulator->m_Controllers[0] |= 0x40;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        nesEmulator->m_Controllers[0] |= 0x20;
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        nesEmulator->m_Controllers[0] |= 0x10;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        nesEmulator->m_Controllers[0] |= 0x08;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        nesEmulator->m_Controllers[0] |= 0x04;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        nesEmulator->m_Controllers[0] |= 0x02;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        nesEmulator->m_Controllers[0] |= 0x01;
    }
}
