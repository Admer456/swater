
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
    void RunGui();

    bool CreateGui();
    bool CreateShaders();
    bool ReloadShaders();
    bool CreateTexture();
    bool CreateGeometry();

    const char* GetShaderError( GLuint vs, GLuint fs ) const;

    int Shutdown( const int& errorCode );

private:
    SDL_Window* window{ nullptr };
    SDL_GLContext glContext{ nullptr };
    ImGuiContext* guiContext{ nullptr };
    bool initialisedGui{ false };

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

    int upperIndex{ 192 };
    int lowerIndex{ 20 };
    GLuint upperIndexHandle{ 0 };
    GLuint lowerIndexHandle{ 0 };

    GLuint textureWidthHandle{ 0 };
    GLuint textureHeightHandle{ 0 };

    GLuint vertexBufferHandle{ 0 };
    GLuint vertexArrayHandle{ 0 };
    GLuint indexBufferHandle{ 0 };
};

/*
Copyright (c) 2022 Admer456

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

