
#pragma once

#include "IApp.hpp"

#define GLEW_STATIC 1
#include <GL/glew.h>

class App final : public IApp
{
public:
    int Run() override;

private:
    void RunFrame();

    bool CreateShaders();
    bool ReloadShaders();
    bool CreateTexture();
    bool CreateGeometry();

    const char* GetShaderError( GLuint vs, GLuint fs ) const;

    int Shutdown( const int& errorCode );

private:
    SDL_Window* window{ nullptr };
    SDL_GLContext glContext{ nullptr };

    bool run{ true };

private:
    GLuint gpuProgramHandle{ 0 };
    GLuint shaderTimeHandle{ 0 };

    // The palette is uploaded to the GPU as a 256x1 texture
    // so we don't have to abuse uniforms
    Texture paletteTexture;
    // The texture itself is uploaded as 8-bit indices into the palette
    Texture texture;

    GLuint paletteTextureHandle{ 0 };
    GLuint textureHandle{ 0 };

    GLuint vertexBufferHandle{ 0 };
    GLuint vertexArrayHandle{ 0 };
    GLuint indexBufferHandle{ 0 };
};
