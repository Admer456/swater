
#pragma once

#include <vector>
#include <array>

//using PaletteEntry = std::array<unsigned char, 3U>;
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
