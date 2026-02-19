// Load PNG (or other image) from file into OpenGL texture for ImGui. One-shot impl.
#include "texture_loader.h"
#include <stdio.h>
#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <GL/gl3w.h>

ImTextureID LoadTextureFromFileOpenGL(const char* path)
{
    if (!path || !*path)
        return (ImTextureID)0;

    int w = 0, h = 0, channels = 0;
    unsigned char* data = stbi_load(path, &w, &h, &channels, 4);
    if (!data || w <= 0 || h <= 0)
    {
        if (data)
            stbi_image_free(data);
        return (ImTextureID)0;
    }

    GLuint tex = 0;
    glGenTextures(1, &tex);
    if (tex == 0)
    {
        stbi_image_free(data);
        return (ImTextureID)0;
    }

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);
    return (ImTextureID)(intptr_t)tex;
}

void ReleaseTextureOpenGL(ImTextureID id)
{
    if (!id)
        return;
    GLuint tex = (GLuint)(intptr_t)id;
    glDeleteTextures(1, &tex);
}
