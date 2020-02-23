// Copyright (c) 2020 Emmanuel Arias
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <chrono>
#include <thread>

#include "include/bus.h"
#include "include/cartridge.h"
#include "include/cpu.h"
#include "include/cpu_widget.h"
#include "include/nes_widget.h"
#include "include/shader.h"
#include "include/file_manager.h"
#include "include/sprite.h"
#include "include/logger.h"
#include "include/nes.h"

using Bus = cpuemulator::Bus;
using Shader = cpuemulator::Shader;
using FileManager = cpuemulator::FileManager;
using Cartridge = cpuemulator::Cartridge;
using Ppu = cpuemulator::Ppu;
using Cpu = cpuemulator::Cpu;
using CpuWidget = cpuemulator::CpuWidget;
using NesWidget = cpuemulator::NesWidget;
using Logger = cpuemulator::Logger;

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, std::shared_ptr<Bus>& nes);

using fsm_handle = Switch;

int main(void) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

    Shader spriteShader;
    spriteShader.Init(FileManager::ReadShader("simple-shader.vs"),
                      FileManager::ReadShader("simple-shader.fs"));

    unsigned int VAO = 0;
    glGenVertexArrays(1, &VAO);

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(NULL);
    ImGui::StyleColorsDark();

    std::shared_ptr<Cartridge> cartridge =
        std::make_shared<Cartridge>("dk.nes");
    if (!cartridge->IsLoaded()) {
        return 1;
    }

    std::shared_ptr<Bus> nesEmulator =
        std::make_shared<Bus>();
    std::shared_ptr<Cpu> cpu = nesEmulator->GetCpuReference();
    std::shared_ptr<Ppu> ppu = nesEmulator->GetPpuReference();

	// todo this will be not needed
    ppu->SetShader(&spriteShader);
    ppu->SetVAO(VAO);

    nesEmulator->InsertCatridge(cartridge);

    CpuWidget cpuWidget{cpu};
    NesWidget nesWidget{nesEmulator};

    cpu->Reset();

    do {
        nesEmulator->Clock();
    } while (!cpu->InstructionComplete());

    glm::mat4 projection = glm::ortho(0.0f, (GLfloat)screenWidth,
                                      (GLfloat)screenHeight, 0.0f, -1.0f, 1.0f);

	Logger& logger = Logger::Get();
    logger.Start();

	Toggle toggle;

    fsm_handle::start();

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
        cpuWidget.Render();
        nesWidget.Render();

        if (nesWidget.IsSimulationRunChecked()) {
            //fsm_handle::dispatch(toggle);
            do {
                nesEmulator->Clock();
            } while (!nesEmulator->GetPpuReference()->isFrameComplete);

            do {
                nesEmulator->GetCpuReference()->Clock();
            } while (nesEmulator->GetCpuReference()->InstructionComplete());

            nesEmulator->GetPpuReference()->isFrameComplete = false;
        } else {
            if (nesWidget.IsDoResetButtonClicked()) {
                nesEmulator->Reset();
            }
            if (nesWidget.IsDoFrameButtonClicked()) {
                do {
                    nesEmulator->Clock();
                } while (!nesEmulator->GetPpuReference()->isFrameComplete);

                do {
                    nesEmulator->GetCpuReference()->Clock();
                } while (nesEmulator->GetCpuReference()->InstructionComplete());

                nesEmulator->GetPpuReference()->isFrameComplete = false;
            }
            if (nesWidget.IsDoStepButtonClicked()) {
                do {
                    nesEmulator->Clock();
                } while (
                    !nesEmulator->GetCpuReference()->InstructionComplete());

                do {
                    nesEmulator->Clock();
                } while (nesEmulator->GetCpuReference()->InstructionComplete());
            }
        }
		ppu->Update();

		// Render sprites
        spriteShader.Use();
        spriteShader.SetUniform("projection", glm::value_ptr(projection));
        ppu->Render();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        milliseconds endFrameTime =
            duration_cast<milliseconds>(system_clock::now().time_since_epoch());

        milliseconds sleepTime = frameTime - (endFrameTime - startFrameTime);
        std::this_thread::sleep_for(sleepTime);
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

void processInput(GLFWwindow* window,
                  std::shared_ptr<Bus>& nesEmulator) {
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
