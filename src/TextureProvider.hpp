
#pragma once

#include <vector>
#include <array>
#include <cstdint>

using PaletteEntry = uint8_t[3];
using PaletteBuffer = std::array<PaletteEntry, 256U>;
using TextureBuffer = std::vector<unsigned char>;

class Texture final
{
public:
    Texture()
    {
        width = 0;
        height = 0;
        buffer = {};
        palette = {};
    }

    Texture( const uint32_t& w, const uint32_t& h, const TextureBuffer& b, const PaletteBuffer& pb )
        : width(w), height(h), buffer(b), palette(pb)
    {
        
    }

    const uint32_t& GetWidth() const
    {
        return width;
    }

    const uint32_t& GetHeight() const
    {
        return height;
    }

    const TextureBuffer& GetBuffer() const
    {
        return buffer;
    }

    const PaletteBuffer GetPalette() const
    {
        return palette;
    }

    operator bool() const
    {
        return width != 0;
    }

private:
    uint32_t width;
    uint32_t height;
    TextureBuffer buffer;
    PaletteBuffer palette;
};

class TextureProvider final
{
public:
    static Texture LoadTextureFromFile( const char* path );
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
