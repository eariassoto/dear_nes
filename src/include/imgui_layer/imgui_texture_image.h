// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <glad/glad.h>

#include <string>

class ImGuiTextureImage {
   public:
    ImGuiTextureImage(unsigned int width, unsigned int height);
    ~ImGuiTextureImage();

    void Update();

    void Render();

    void SetPixel(int x, int y, int color);

    void CopyTextureFromArray(const int* intArray);

    void ScaleImageToWidth(float newWidth);

    void ScaleImageToHeight(float newHeight);

    float GetTextureWidth() const;
    float GetTextureHeight() const;

   private:
    unsigned int m_Width = 0;
    unsigned int m_Height = 0;

    float m_TextureWidth = 0;
    float m_TextureHeight = 0;

    GLubyte* m_TextureData = nullptr;
    GLuint m_textureId = 0;

    static constexpr GLenum PIXEL_FORMAT = GL_BGRA;
    static constexpr int CHANNEL_COUNT = 4;
};
