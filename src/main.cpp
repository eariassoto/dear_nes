// Copyright (c) 2020 Emmanuel Arias
// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
// clang-format on
#include <iostream>
#include <memory>
#include <sstream>
#include <chrono>
#include <thread>

#include "include/nes.h"
#include "include/cartridge.h"
#include "include/cpu_widget.h"
#include "include/file_manager.h"
#include "include/sprite.h"
#include "include/logger.h"
#include "include/ui_config.h"

using Nes = cpuemulator::Nes;
using FileManager = cpuemulator::FileManager;
using Cartridge = cpuemulator::Cartridge;
using Ppu = cpuemulator::Ppu;
using Cpu = cpuemulator::Cpu;
using CpuWidget = cpuemulator::CpuWidget;
using Logger = cpuemulator::Logger;
using UiConfig = cpuemulator::UiConfig;

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, std::shared_ptr<Nes>& nes);

void RenderUISettingsWidget(UiConfig* uiConfig) {
    uiConfig->ResetEvents();

    ImGui::Begin("UI Settings");
    ImGui::SetWindowSize({512, 80});
    ImGui::SetWindowPos({10, 540}, ImGuiCond_Once);

    if (uiConfig->m_EmulatorIsRunning) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha,
                            ImGui::GetStyle().Alpha * 0.5f);
        ImGui::Button("Run");
        ImGui::PopStyleVar();
    } else {
        uiConfig->m_EmulatorIsRunning = ImGui::Button("Run");
    }

    ImGui::SameLine();

    if (uiConfig->m_EmulatorIsRunning) {
        if (ImGui::Button("Pause")) {
            uiConfig->m_EmulatorIsRunning = false;
        }
    } else {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha,
                            ImGui::GetStyle().Alpha * 0.5f);
        ImGui::Button("Pause");
        ImGui::PopStyleVar();
    }

    ImGui::SameLine();

    if (ImGui::Button("Reset")) {
        uiConfig->m_EmulatorIsRunning = false;
        uiConfig->m_EmulatorMustReset = true;
    }

	ImGui::Checkbox("Pattern Table #0", &uiConfig->m_PpuShowPatternTable0);
    ImGui::SameLine();
	ImGui::Checkbox("Pattern Table #1", &uiConfig->m_PpuShowPatternTable1);

    ImGui::End();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage nes-emulator.exe [path/to/rom]\n";
        return 1;
    }

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    UiConfig uiConfig;
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
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(NULL);
    ImGui::StyleColorsDark();

    std::shared_ptr<Cartridge> cartridge = std::make_shared<Cartridge>(argv[1]);
    if (!cartridge->IsLoaded()) {
        return 1;
    }

    std::shared_ptr<Nes> nesEmulator = std::make_shared<Nes>(uiConfig);

    nesEmulator->InsertCatridge(cartridge);

    nesEmulator->Reset();

    Logger& logger = Logger::Get();
    logger.Start();

    using namespace std::chrono;
    const milliseconds frameTime{1000 / 60};
    while (!glfwWindowShouldClose(window)) {
        std::chrono::milliseconds startFrameTime =
            duration_cast<milliseconds>(system_clock::now().time_since_epoch());

        glfwPollEvents();

        processInput(window, nesEmulator);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // render widgets
        RenderUISettingsWidget(&uiConfig);
        nesEmulator->RenderWidgets();

        nesEmulator->DoFrame();

        nesEmulator->Update();

        nesEmulator->Render();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        const milliseconds endFrameTime =
            duration_cast<milliseconds>(system_clock::now().time_since_epoch());
        const milliseconds delta = (endFrameTime - startFrameTime);

        if (delta < frameTime) {
            std::this_thread::sleep_for(frameTime - delta);
        } else {
            std::cout << "Frame took longer than 1000/60 ms\n";
        }
    }

    logger.Stop();
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window, std::shared_ptr<Nes>& nesEmulator) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    nesEmulator->m_Controllers[0] = 0x00;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        nesEmulator->m_Controllers[0] |= 0x80;
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        nesEmulator->m_Controllers[0] |= 0x40;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        nesEmulator->m_Controllers[0] |= 0x20;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
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
