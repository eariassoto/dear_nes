// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <glad/glad.h>

#include <string>

class ImGuiTextureImage {
   public:
    ImGuiTextureImage(unsigned int width, unsigned int height,
                      unsigned int cellSize);
    ~ImGuiTextureImage();

    void Update();

    void Render();

    void SetPixel(int x, int y, int color);

    void CopyTextureFromArray(const int* intArray);

   private:
    unsigned int m_Width = 0;
    unsigned int m_Height = 0;

    unsigned int m_CellSizeInPixels = 0;

    float m_TextureWidth = 0;
    float m_TextureHeight = 0;

    GLubyte* m_TextureData = nullptr;
    GLuint m_textureId = 0;

    static constexpr GLenum PIXEL_FORMAT = GL_BGRA;
    static constexpr int CHANNEL_COUNT = 4;
};
