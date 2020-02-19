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

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, cpuemulator::Bus& bus);

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
        std::make_shared<cpuemulator::Cartridge>("palette.nes");
    if (!cartridge->IsLoaded()) {
        return 1;
    }

    cpuemulator::Bus bus;
    bus.InsertCatridge(cartridge);

    cpuemulator::CpuWidget cpuWidget{bus.m_Cpu};
    cpuemulator::NesWidget nesWidget{bus};

    bus.m_Ppu.m_SpriteScreen.BindToVAO(VAO);
    bus.m_Ppu.m_SpritePatternTable[0].BindToVAO(VAO);
    bus.m_Ppu.m_SpritePatternTable[1].BindToVAO(VAO);

    bus.m_Cpu.Reset();

    do {
        bus.Clock();
    } while (!bus.m_Cpu.InstructionComplete());

    glm::mat4 projection = glm::ortho(0.0f, (GLfloat)screenWidth,
                                      (GLfloat)screenHeight, 0.0f, -1.0f, 1.0f);

    Sprite& patternTable1 = bus.m_Ppu.GetPatternTable(0, 2);
    Sprite& patternTable2 = bus.m_Ppu.GetPatternTable(1, 2);

    Sprite palette{ 9, 4, 30, 532, 300 };
    palette.BindToVAO(VAO);

    using namespace std::chrono;
    const milliseconds frameTime{1000 / 60};
    while (!glfwWindowShouldClose(window)) {
        std::chrono::milliseconds startFrameTime =
            duration_cast<milliseconds>(system_clock::now().time_since_epoch());

        glfwPollEvents();

        processInput(window, bus);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render sprites
        spriteShader.Use();
        spriteShader.SetUniform("projection", glm::value_ptr(projection));
        bus.m_Ppu.m_SpriteScreen.Render(spriteShader);

        patternTable1 = bus.m_Ppu.GetPatternTable(0, 0);
        patternTable2 = bus.m_Ppu.GetPatternTable(1, 0);

        patternTable1.Render(spriteShader);
        patternTable2.Render(spriteShader);

        for (int p = 0; p < 8; ++p) // For each palette
        {
            for (int s = 0; s < 4; ++s) // For each index
            {
                const int coordX = (p > 3) ? s + 5: s;
                const int coordY = p % 4;
                palette.SetPixel(coordX, coordY, bus.m_Ppu.GetColorFromPalette(p, s));
            }
        }
        palette.Render(spriteShader);
            
        // render widgets
        cpuWidget.Render();
        nesWidget.Render();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        milliseconds endFrameTime =
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

void processInput(GLFWwindow* window, cpuemulator::Bus& bus) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    bus.m_Controllers[0] = 0x00;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        bus.m_Controllers[0] |= 0x80;
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        bus.m_Controllers[0] |= 0x40;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        bus.m_Controllers[0] |= 0x20;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        bus.m_Controllers[0] |= 0x10;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        bus.m_Controllers[0] |= 0x08;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        bus.m_Controllers[0] |= 0x04;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        bus.m_Controllers[0] |= 0x02;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        bus.m_Controllers[0] |= 0x01;
    }
}
