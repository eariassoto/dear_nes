// Copyright (c) 2020 Emmanuel Arias
#include <cstring>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "include/sprite.h"
#include "include/shader.h"
#include "include/logger.h"

namespace cpuemulator {
Sprite::Sprite(unsigned int width, unsigned int height, float cellSize, float posX, float posY)
    : m_Width{width},
      m_Height{height},
    m_CellSizeInPixels{cellSize},
    m_PositionX{posX},
    m_PositionY{posY},
    m_ModelMatrix{ glm::mat4(1.0f) } {
    int dataSize = m_Width * m_Height * CHANNEL_COUNT;
    m_TextureData = new GLubyte[dataSize];
    memset(m_TextureData, 0, dataSize);

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

    float spriteWidth = m_Width * m_CellSizeInPixels;
    float spriteHeight = m_Height * m_CellSizeInPixels;
    m_ModelMatrix = glm::translate(m_ModelMatrix, glm::vec3(m_PositionX, m_PositionY, 0.0f));
    m_ModelMatrix = glm::scale(m_ModelMatrix, glm::vec3(spriteWidth, spriteHeight, 1.0f));
}

Sprite::~Sprite() {
    delete[] m_TextureData;
    glDeleteTextures(1, &m_textureId);
}
void Sprite::BindToVAO(unsigned int VAO) {
    m_VAO = VAO;
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TRIANGLE_VERTICES),
        TRIANGLE_VERTICES, GL_STATIC_DRAW);

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

void Sprite::Render(Shader& shader) {
    shader.Use();
    shader.SetUniform("model", glm::value_ptr(m_ModelMatrix));

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

void Sprite::SetPixel(int x, int y, int color)
{
    if (x < 0 || x >= m_Width || y < 0 || y >= m_Height)
    {
        return;
    }

    int* ptr = reinterpret_cast<int*>(m_TextureData);
    const int position = (y * m_Width) + x;
    ptr[position] = color;
}

}  // namespace cpuemulator