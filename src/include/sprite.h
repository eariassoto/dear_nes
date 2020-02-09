// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace cpuemulator {

class Sprite {
   public:
    Sprite(unsigned int width, unsigned int height);
    ~Sprite();

	void BindToVAO(unsigned int VAO);
    void Render();

   private:
    unsigned int m_Width = 0;
    unsigned int m_Height = 0;
    float m_TriangleVertices[20] = {};
    GLubyte* m_TextureData = nullptr;
    GLuint m_textureId = 0;
    unsigned int m_VAO = 0;
    unsigned int m_VBO = 0;
    unsigned int m_EBO = 0;

	static constexpr GLenum PIXEL_FORMAT = GL_BGRA;
    static constexpr int CHANNEL_COUNT = 4;
    static constexpr unsigned int TRIANGLE_INDEXES[] = {
        0, 1, 3,  // first triangle
        1, 2, 3   // second triangle
    };

	void PaintChessBoard();
};
}  // namespace cpuemulator
