
#include "TextureProvider.hpp"

#define STBI_ONLY_BMP 1
#define STB_IMAGE_IMPLEMENTATION 1
#include "stb_image.h"

#include <iostream>
#include <fstream>

class FileReader final
{
public:
	FileReader( const char* path )
	{
		file = std::ifstream( path, std::ifstream::binary );
	}

	bool IsOkay() const
	{
		return !!file;
	}

	operator bool() const
	{
		return IsOkay();
	}

	template< typename T = unsigned char >
	T Read( const bool& debug = false )
	{
		T temp{};
		file.read( reinterpret_cast<char*>(&temp), sizeof(T) );
		
		if ( debug )
		{
			std::cout << "FileReader::Read: " << (uint32_t)temp << " at 0x" << std::hex << position << std::endl;
		}

		if ( file.eof() )
		{
			std::cout << "EOF EOF EOF at 0x" << std::hex << position << std::endl;
		}

		if ( !IsOkay() )
		{
			std::cout << "Oh shit, failed somehow!!! (at 0x" << std::hex << position << ")" << std::endl;
		}

		position += sizeof(T);

		return temp;
	}
	
	uint32_t GetPosition() const
	{
		return position;
	}

private:
	std::ifstream file;
	uint32_t position{ 0 };
};

Texture TextureProvider::LoadTextureFromFile( const char* path )
{
	FileReader file( path );

	if ( !file )
	{
		std::cout << "The image does not exist" << std::endl;
		return Texture{};
	}

	if ( file.Read() != 'B' || file.Read() != 'M' )
	{
		std::cout << "The image isn't a BMP" << std::endl;
		return Texture();
	}

	file.Read<uint32_t>(); // Discard filesize
	file.Read<uint16_t>(); // Discard reserved
	file.Read<uint16_t>(); // Discard reserved

	std::cout << "offset: 0x" << std::hex << file.GetPosition() << std::endl;
	auto offset = file.Read<int32_t>();
	std::cout << "headerSize: 0x" << std::hex << file.GetPosition() << std::endl;
	auto headerSize = file.Read<int32_t>();
	auto extra_read = 14;

	// We expect the header size to be 40 bytes
	if ( headerSize != 40 )
	{
		std::cout << "The image isn't an 8-bit thingy, pls use IrfanView to index :(" << std::endl;
		return Texture{};
	}

	if ( offset < 0 )
	{
		std::cout << "Bad BMP, very bad BMP >:(" << std::endl;
		return Texture{};
	}

	std::cout << "x: 0x" << std::hex << file.GetPosition() << std::endl;
	auto x = file.Read<uint32_t>();
	std::cout << "y: 0x" << std::hex << file.GetPosition() << std::endl;
	auto y = file.Read<uint32_t>();

	if ( x & 15 || y & 15 )
	{
		std::cout << "Texture not divisible by 16" << std::endl;
		return Texture{};
	}

	// The 2 bytes after the dimension should be 01
	if ( file.Read<uint16_t>() != 1 )
	{
		std::cout << "Bad BMP, very bad BMP >:(" << std::endl;
		return Texture{};
	}

	std::cout << "bpp: 0x" << std::hex << file.GetPosition() << std::endl;
	// We expect bits per pixel to be 8
	auto bpp = file.Read<int16_t>();
	if ( bpp != 8 )
	{
		std::cout << "The image isn't an 8-bit thingy, pls use IrfanView to index :(" << std::endl;
		return Texture{};
	}

	std::cout << "Dimensions: " << x << "x" << y << std::endl
		<< "Bits per pimxel: " << bpp << std::endl;

	std::cout << "compress: 0x" << std::hex << file.GetPosition() << std::endl;
	// We expect compress to be 0
	auto compress = file.Read<int16_t>();
	if ( compress )
	{
		std::cout << "The image isn't an 8-bit uncompressed thingy, pls use IrfanView :)" << std::endl;
		return Texture{};
	}

	// Skip sizeof, hres
	file.Read<uint64_t>();
	// Skip vres, colorsused
	file.Read<uint64_t>();
	// Skip max important
	file.Read<uint32_t>();

	int paletteSize = (offset - extra_read - headerSize) >> 2;
	if ( paletteSize != 256 )
	{
		std::cout << "Corrupt BMP, bad offset or palette size is different than 256: " << paletteSize << std::endl;
		return Texture{};
	}

	PaletteBuffer paletteBuffer{};
	TextureBuffer textureBuffer{};
	textureBuffer.reserve( x * y );

	std::cout << "Palette starts at: 0x" << std::hex << file.GetPosition() << std::endl;

	// Read 2 more bytes here, I noticed something wrong when I was looking at it in my hex editor :>
	file.Read<uint16_t>();

	// Palette loading
	for ( auto& paletteEntry : paletteBuffer )
	{
		paletteEntry[2] = file.Read<uint8_t>();
		paletteEntry[1] = file.Read<uint8_t>();
		paletteEntry[0] = file.Read<uint8_t>();
		file.Read(); // skip alpha it seems
	}
	std::cout << "Palette ends at: 0x" << std::hex << file.GetPosition() << std::endl;

	// Skip a number of bytes
	{
		uint32_t skipRange = offset - extra_read - headerSize - paletteSize * 4;
		std::cout
			<< "offset: " << offset << std::endl
			<< "extra_read: " << extra_read << std::endl
			<< "headerSize: " << headerSize << std::endl
			<< "paletteSize: " << paletteSize << std::endl
			<< "skipRange: " << skipRange << std::endl;

		std::cout << "Before skipping: 0x" << std::hex << file.GetPosition() << std::endl;
		for ( uint32_t i = 0U; i < skipRange; i++ )
		{
			file.Read();
		}
		std::cout << "After skipping: 0x" << std::hex << file.GetPosition() << std::endl;
	}

	// Read le indices into le paletteuatiouei
	for ( int i = 0; i < (x * y); i++ )
	{
		textureBuffer.push_back( file.Read() );	
	}

	std::cout << "Success" << std::endl;
	return Texture( x, y, textureBuffer, paletteBuffer );
}

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

