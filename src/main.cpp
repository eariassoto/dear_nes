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

#include "include/bus.h"
#include "include/cartridge.h"
#include "include/cpu.h"
#include "include/cpu_widget.h"
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

    GLFWwindow* window =
        glfwCreateWindow(1000, 650, "NES Emulator", NULL, NULL);
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

    glViewport(0, 0, 1250, 600);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	Shader triangleShader;
    triangleShader.Init(FileManager::ReadShader("simple-shader.vs"),
                         FileManager::ReadShader("simple-shader.fs"));

	unsigned int VAO = 0;
	glGenVertexArrays(1, &VAO);

    Sprite sprite{8, 8};
    sprite.BindToVAO(VAO);

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

    bus.CpuWrite(0xFFFC, 0x00);
    bus.CpuWrite(0xFFFD, 0x80);

    bus.m_Cpu.Reset();

	do {
        bus.m_Cpu.Clock();
    } while (!bus.m_Cpu.InstructionComplete());

	static glm::mat4 trans = glm::mat4(1.0f);
    trans = glm::scale(trans, glm::vec3(0.5, 0.5, 0.5));

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

		// Render sprites
		triangleShader.Use();
        triangleShader.SetUniform("transform", glm::value_ptr(trans));
        sprite.Render();

        // render widgets
        cpuWidget.Render();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
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
