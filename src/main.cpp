#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <sstream>

#include "include/bus.h"
#include "include/cpu.h"
#include "include/cpu_widget.h"
#include "include/instruction_disassembler.h"
#include "include/ram_widget.h"

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

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

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(NULL);
    ImGui::StyleColorsDark();

    cpuemulator::Bus bus;
    cpuemulator::Cpu cpu;
    cpu.ConnectBus(&bus);

    cpuemulator::CpuWidget cpuWidget{cpu};

    void* ramSection1Ptr = (void*)(bus.GetMemoryPtr(0x0000));
    void* ramSection2Ptr = (void*)(bus.GetMemoryPtr(0x8000));
    cpuemulator::RamWidget ramWidget1{"Memory Editor 1", ramSection1Ptr, 0xFF,
                                      0x0000};
    cpuemulator::RamWidget ramWidget2{"Memory Editor 2", ramSection2Ptr, 0xFF,
                                      0x8000};

    cpuemulator::InstructionDisassembler instr{bus};
    // Convert hex string into bytes for RAM
    std::stringstream ss;
    ss << "A2 0A 8E 00 00 A2 03 8E 01 00 AC 00 00 A9 00 18 6D 01 00 88 D0 FA "
          "8D 02 00 EA EA EA";
    uint16_t nOffset = 0x8000;
    while (!ss.eof()) {
        std::string b;
        ss >> b;
        bus.Write(nOffset++, (uint8_t)std::stoul(b, nullptr, 16));
    }

    instr.DisassembleMemory(0x8000, 0x80F0);

    // Set Reset Vector
    bus.Write(0xFFFC, 0x00);
    bus.Write(0xFFFD, 0x80);

    cpu.Reset();
    do {
        cpu.Clock();
    } while (!cpu.InstructionComplete());

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // render widgets
        cpuWidget.Render();
        ramWidget1.Render();
        ramWidget2.Render();
        instr.Render(cpu.GetProgramCounter());

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
