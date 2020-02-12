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
#include <thread>
#include <chrono>

#include "include/bus.h"
#include "include/cartridge.h"
#include "include/cpu.h"
#include "include/cpu_widget.h"
#include "include/nes_widget.h"
#include "include/shader.h"
#include "include/file_manager.h"
#include "include/sprite.h"

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

using cpuemulator::FileManager;
using cpuemulator::Shader;
using cpuemulator::Sprite;

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

    std::shared_ptr<cpuemulator::Cartridge> cartridge =
        std::make_shared<cpuemulator::Cartridge>("nestest.nes");
    if (!cartridge->IsLoaded()) {
        return 1;
    }

    cpuemulator::Bus bus;
    bus.InsertCatridge(cartridge);

    cpuemulator::CpuWidget cpuWidget{bus.m_Cpu};
    cpuemulator::NesWidget nesWidget{bus};

    bus.CpuWrite(0xFFFC, 0x00);
    bus.CpuWrite(0xFFFD, 0x80);

    bus.m_Ppu.m_SpriteScreen.BindToVAO(VAO);

    bus.m_Cpu.Reset();

    do {
        bus.m_Cpu.Clock();
    } while (!bus.m_Cpu.InstructionComplete());

    glm::mat4 projection = glm::ortho(0.0f, (GLfloat)screenWidth,
                                      (GLfloat)screenHeight, 0.0f, -1.0f, 1.0f);

    using namespace std::chrono;
    const milliseconds frameTime{1000 / 60};

    while (!glfwWindowShouldClose(window)) {
        std::chrono::milliseconds startFrameTime =
            duration_cast<milliseconds>(system_clock::now().time_since_epoch());

        glfwPollEvents();

        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ///
        if (true) {
            do {
                bus.Clock();
            } while (!bus.m_Ppu.isFrameComplete);

            do {
                bus.m_Cpu.Clock();
            } while (bus.m_Cpu.InstructionComplete());
            bus.m_Ppu.isFrameComplete = false;
        }
        ////

        // Render sprites
        spriteShader.Use();

        spriteShader.SetUniform("projection", glm::value_ptr(projection));
        bus.m_Ppu.m_SpriteScreen.Render(spriteShader);

        // render widgets
        cpuWidget.Render();
        nesWidget.Render();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        std::chrono::milliseconds endFrameTime =
            duration_cast<milliseconds>(system_clock::now().time_since_epoch());

        milliseconds sleepTime = frameTime - (endFrameTime - startFrameTime);

        std::this_thread::sleep_for(sleepTime);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}
