// Copyright (c) 2020 Emmanuel Arias
#include "include/imgui_layer/imgui_texture_image.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <cstring>
#include <string>

ImGuiTextureImage::ImGuiTextureImage(unsigned int width, unsigned int height)
    : m_Width{width},
      m_Height{height},
      m_TextureWidth{static_cast<float>(width)},
      m_TextureHeight{static_cast<float>(height)} {
    int dataSize = m_Width * m_Height * CHANNEL_COUNT;
    m_TextureData = new GLubyte[dataSize];
    memset(m_TextureData, 0, dataSize);

    // Logger::Get().Log("SPRITE", "Created {}x{} sprite", m_Width, m_Height);

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
    // Logger::Get().Log("SPRITE", "Sprite binded to texture id {}", m_textureId);
}

ImGuiTextureImage::~ImGuiTextureImage() {
    delete[] m_TextureData;
    glDeleteTextures(1, &m_textureId);
}

void ImGuiTextureImage::Update() {
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height, PIXEL_FORMAT,
                    GL_UNSIGNED_BYTE, (GLvoid*)m_TextureData);
}

void ImGuiTextureImage::Render() {
    ImGui::Image((void*)(intptr_t)m_textureId,
                 ImVec2(m_TextureWidth, m_TextureHeight));
}

void ImGuiTextureImage::SetPixel(int x, int y, int color) {
    if (x < 0 || x >= static_cast<int>(m_Width) || y < 0 ||
        y >= static_cast<int>(m_Height)) {
        return;
    }

    int* ptr = reinterpret_cast<int*>(m_TextureData);
    const int position = (y * m_Width) + x;
    ptr[position] = color;
}

void ImGuiTextureImage::CopyTextureFromArray(const int* intArray) {
    memcpy(reinterpret_cast<void*>(m_TextureData),
           reinterpret_cast<const void*>(intArray),
           m_Width * m_Height * sizeof(int));
}

void ImGuiTextureImage::ScaleImageToWidth(float newWidth) {
    if (newWidth == m_TextureWidth) {
        return;
    }
    float scale = newWidth / m_TextureWidth;
    m_TextureWidth = newWidth;
    m_TextureHeight *= scale;
}

void ImGuiTextureImage::ScaleImageToHeight(float newHeight) {
    if (newHeight == m_TextureHeight) {
        return;
    }
    float scale = newHeight / m_TextureHeight;
    m_TextureWidth *= scale;
    m_TextureHeight = newHeight;
}

float ImGuiTextureImage::GetTextureWidth() const { return m_TextureWidth; }

float ImGuiTextureImage::GetTextureHeight() const { return m_TextureHeight; }
