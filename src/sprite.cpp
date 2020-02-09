// Copyright (c) 2020 Emmanuel Arias
#include <cstring>

#include "include/sprite.h"
#include "include/logger.h"

namespace cpuemulator {
Sprite::Sprite(unsigned int width, unsigned int height)
    : m_Width{width},
      m_Height{height},
      m_TriangleVertices{
          // positions          // texture coords
          1.0f,  1.0f,  0.0f, 1.0f, 1.0f,  // top right
          1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
          -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // bottom left
          -1.0f, 1.0f,  0.0f, 0.0f, 1.0f   // top left
      } {
    int dataSize = m_Width * m_Height * CHANNEL_COUNT;
    m_TextureData = new GLubyte[dataSize];
    memset(m_TextureData, 0, dataSize);
    PaintChessBoard();

    Logger::Get().Log("SPRITE", "Created {}x{} sprite", m_Width, m_Height);

    glGenTextures(1, &m_textureId);
    // TODO check error

    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, PIXEL_FORMAT,
                 GL_UNSIGNED_BYTE, (GLvoid*)m_TextureData);
    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
    Logger::Get().Log("SPRITE", "Sprite binded to texture id {}", m_textureId);

    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);
}

Sprite::~Sprite() {
    delete[] m_TextureData;
    glDeleteTextures(1, &m_textureId);
}
void Sprite::BindToVAO(unsigned int VAO) {
    m_VAO = VAO;
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_TriangleVertices),
                 m_TriangleVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(TRIANGLE_INDEXES),
                 TRIANGLE_INDEXES, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

	glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Sprite::Render() {
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height,
                    PIXEL_FORMAT, GL_UNSIGNED_BYTE, (GLvoid*)m_TextureData);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Sprite::PaintChessBoard()
{
    if (m_TextureData == nullptr) return;

    int* ptr = reinterpret_cast<int*>(m_TextureData);
    // copy 4 bytes at once
    for (int i = 0; i < m_Height; ++i) {
        for (int j = 0; j < m_Width; ++j, ++ptr) {
            if (i % 2 == j % 2) {
                *ptr = 0x005D6D7E;
            } else {
                *ptr = 0x00FF5733;
            }
        }
    }
}

}  // namespace cpuemulator