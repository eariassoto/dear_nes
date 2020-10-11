// Copyright (c) 2020 Emmanuel Arias
#include "include/sprite.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <cstring>
#include <string>

#include "include/logger.h"

namespace cpuemulator {
Sprite::Sprite(const std::string& spriteName, unsigned int width,
               unsigned int height, unsigned int cellSize, float posX,
               float posY)
    : m_Name{spriteName},
      m_Width{width},
      m_Height{height},
      m_CellSizeInPixels{cellSize},
      m_TextureWidth{static_cast<float>(width) * cellSize},
      m_TextureHeight{static_cast<float>(height) * cellSize},
      m_PositionX{posX},
      m_PositionY{posY} {
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
}

Sprite::~Sprite() {
    delete[] m_TextureData;
    glDeleteTextures(1, &m_textureId);
}

void Sprite::Update() {
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height, PIXEL_FORMAT,
                    GL_UNSIGNED_BYTE, (GLvoid*)m_TextureData);
}

void Sprite::Render() {
    ImGui::SetNextWindowPos(ImVec2(m_PositionX, m_PositionY), ImGuiCond_Once);
    ImGui::Begin(m_Name.c_str());
    ImGui::Image((void*)(intptr_t)m_textureId,
                 ImVec2(m_TextureWidth, m_TextureHeight));
    ImGui::End();
}

void Sprite::SetPixel(int x, int y, int color) {
    if (x < 0 || x >= static_cast<int>(m_Width) || y < 0 ||
        y >= static_cast<int>(m_Height)) {
        return;
    }

    int* ptr = reinterpret_cast<int*>(m_TextureData);
    const int position = (y * m_Width) + x;
    ptr[position] = color;
}

void Sprite::CopyTextureFromArray(const int* intArray) {
    memcpy(reinterpret_cast<void*>(m_TextureData),
           reinterpret_cast<const void*>(intArray), m_Width * m_Height * sizeof(int));
}

}  // namespace cpuemulator