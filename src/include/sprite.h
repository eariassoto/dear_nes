// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace cpuemulator {

class Shader;

class Sprite {
   public:
    Sprite(unsigned int width, unsigned int height, float cellSize,
           float posX = 0, float posY = 0);
    ~Sprite();

    void BindToVAO(unsigned int VAO);
    void Render(Shader& shader);

    void SetPixel(int x, int y, int color);

   private:
    unsigned int m_Width = 0;
    unsigned int m_Height = 0;
    float m_CellSizeInPixels = 25;
    float m_PositionX = 0;
    float m_PositionY = 0;
    GLubyte* m_TextureData = nullptr;
    GLuint m_textureId = 0;
    unsigned int m_VAO = 0;
    unsigned int m_VBO = 0;
    unsigned int m_EBO = 0;
    glm::mat4 m_ModelMatrix;

    static constexpr GLenum PIXEL_FORMAT = GL_BGRA;
    static constexpr int CHANNEL_COUNT = 4;

    static constexpr float TRIANGLE_VERTICES[] = {
        // positions          // texture coords
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  // top right
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // bottom right
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom left
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f   // top left
    };
    static constexpr unsigned int TRIANGLE_INDEXES[] = {
        0, 1, 3,  // first triangle
        1, 2, 3   // second triangle
    };
};
}  // namespace cpuemulator
