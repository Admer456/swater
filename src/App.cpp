
#include <iostream>

#include "SDL.h"
#include "TextureProvider.hpp"
#include "App.hpp"
#include <sstream>
#include <fstream>

IApp& GetApp()
{
	static App app;
	return app;
}

// Taken from FoxGLBox:
// https://github.com/Admer456/FoxGLBox/blob/master/renderer/src/Backends/OpenGL45/Renderer.cpp#L25
const char* glTranslateError( GLenum error )
{
	// Originally, it was int glEnum, but GCC complains about narrowing conversions
	// This way is more correct anyway
	struct GLErrorInfo { GLenum glEnum; const char* glString; };

	constexpr GLErrorInfo errorEnumStrings[] =
	{
		#define glErrorInfo( glEnum ) { glEnum, #glEnum },
		glErrorInfo( GL_INVALID_ENUM )
		glErrorInfo( GL_INVALID_FRAMEBUFFER_OPERATION )
		glErrorInfo( GL_INVALID_INDEX )
		glErrorInfo( GL_INVALID_VALUE )
		glErrorInfo( GL_INVALID_OPERATION )
		glErrorInfo( GL_STACK_OVERFLOW )
		glErrorInfo( GL_STACK_UNDERFLOW )
		glErrorInfo( GL_OUT_OF_MEMORY )
		#undef glErrorInfo
	};

	for ( const auto& errorInfo : errorEnumStrings )
	{
		if ( error == errorInfo.glEnum )
			return errorInfo.glString;
	}

	return (const char*)glewGetErrorString( error );
}

bool GLError( const char* why = nullptr )
{
	printf( "OpenGL: %s", why );

	auto error = glGetError();
	if ( error != GL_NO_ERROR )
	{
		printf( ", got an error:\n 0x%x | %i (%s)\n_________________________\n",
			error, error, glTranslateError( error ) );
		return true;
	}

	printf( ", went OK\n" );

	return false;
}

int App::Run()
{
	SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS );
	{
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

		SDL_GL_SetSwapInterval( 1 );

		window = SDL_CreateWindow( "SWater", 
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			1024, 1024, SDL_WINDOW_OPENGL );

		glContext = SDL_GL_CreateContext( window );

		auto errorCode = glewInit();
		if ( errorCode != GLEW_OK )
		{
			const unsigned char* errorString = glewGetErrorString( errorCode );
			std::cout << "GLEW Error: " << errorString << std::endl;
			return Shutdown( Failure );
		}
	}

	if ( !CreateShaders() )
		return Shutdown( Failure );

	if ( !CreateTexture() )
		return Shutdown( Failure );

	if ( !CreateGeometry() )
		return Shutdown( Failure );

	while ( run )
	{
		RunFrame();
	}

	return Shutdown( Success );
}

void App::RunFrame()
{
	SDL_Event ev;
	while ( SDL_PollEvent( &ev ) )
	{
		if ( ev.type == SDL_QUIT )
		{
			run = false;
			break;
		}
		// TODO: ImGui buttons'n'stuff so we can select other shaders
		else if ( ev.type == SDL_KEYDOWN )
		{
			if ( ev.key.keysym.scancode == SDL_SCANCODE_R )
			{
				ReloadShaders();
			}
		}
	}

	static float time = 0.0f;
	time += 0.016f;

	glClearColor( 0.05f, 0.15f, 0.15f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// Use the shader
	glUseProgram( gpuProgramHandle );
	
	// Update the time
	glUniform1f( shaderTimeHandle, time );

	// Bind the textures
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, textureHandle );
	glActiveTexture( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_2D, paletteTextureHandle );

	// Render go brr
	glBindVertexArray( vertexArrayHandle );
	glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr );

	SDL_GL_SwapWindow( window );
}

// Just a vertex and fragment shader, nothing special
bool App::CreateShaders()
{
	const auto loadShaderFromFile = []( const char* filePath, std::string& str )
	{
		std::ifstream file( filePath );

		if ( !file )
		{
			std::cout << "CreateShaders: '" << filePath << "' does not exist" << std::endl;
			return false;
		}

		std::string temp;
		while ( std::getline( file, temp ) )
		{
			str += temp + "\n";
		}

		return true;
	};

	std::string vertexShaderCode;
	if ( !loadShaderFromFile( "vertexShader.glsl", vertexShaderCode ) )
	{
		return false;
	}

	std::string fragmentShaderCode;
	if ( !loadShaderFromFile( "pixelShader.glsl", fragmentShaderCode ) )
	{
		return false;
	}

	const char* vertexShaderString = vertexShaderCode.c_str();
	const char* fragmentShaderString = fragmentShaderCode.c_str();

	gpuProgramHandle = glCreateProgram();
	GLuint vertexShaderHandle = glCreateShader( GL_VERTEX_SHADER );
	GLuint fragmentShaderHandle = glCreateShader( GL_FRAGMENT_SHADER );

	glShaderSource( vertexShaderHandle, 1, &vertexShaderString, nullptr );
	glShaderSource( fragmentShaderHandle, 1, &fragmentShaderString, nullptr );

	// Compile le shadeurs
	glCompileShader( vertexShaderHandle );
	glCompileShader( fragmentShaderHandle );

	const char* errorMessage = GetShaderError( vertexShaderHandle, fragmentShaderHandle );
	if ( nullptr != errorMessage )
	{
		std::cout << "Error while compiling: " << errorMessage << std::endl;
		return false;
	}

	glAttachShader( gpuProgramHandle, vertexShaderHandle );
	glAttachShader( gpuProgramHandle, fragmentShaderHandle );
	glLinkProgram( gpuProgramHandle );

	glDeleteShader( vertexShaderHandle );
	glDeleteShader( fragmentShaderHandle );

	shaderTimeHandle = glGetUniformLocation( gpuProgramHandle, "gTime" );

	glUseProgram( gpuProgramHandle );

	GLuint diffuseMapHandle = glGetUniformLocation( gpuProgramHandle, "diffuseMap" );
	GLuint paletteMapHandle = glGetUniformLocation( gpuProgramHandle, "paletteMap" );

	glUniform1i( diffuseMapHandle, 0 );
	glUniform1i( paletteMapHandle, 1 );

	return true;
}

bool App::ReloadShaders()
{
	if ( gpuProgramHandle )
	{
		glDeleteProgram( gpuProgramHandle );
		gpuProgramHandle = 0;
	}
	
	// Let's get a black (or white on some GPUs) quad
	// if the shader is wrong
	if ( !CreateShaders() )
	{
		gpuProgramHandle = 0;
		return false;
	}

	return true;
}

// Also taken from FoxGLBox
// =====================================================================
// PrintTextureInfo
// Debugging helper function, converts some OpenGL enums
// into strings and prints them
// =====================================================================
void PrintTextureInfo( int textureDataType, int textureFormat )
{
	printf( "## TextureInfo:\n" );
	struct TexInfo { int glEnum; const char* glString; };

	constexpr TexInfo infos[] =
	{
		#define texInfo( glEnum ) { glEnum, #glEnum },
		texInfo( GL_UNSIGNED_BYTE )
		texInfo( GL_FLOAT )
		texInfo( GL_RGB )
		texInfo( GL_RGB8 )
		texInfo( GL_RGBA )
		texInfo( GL_RGBA8 )
		texInfo( GL_RGB32F )
		texInfo( GL_RGBA32F )
		texInfo( GL_R )
		texInfo( GL_RED )
		texInfo( GL_R8 )
		texInfo( GL_R32F )
		#undef texInfo
	};

	for ( const auto& info : infos )
	{
		if ( textureDataType == info.glEnum )
		{
			printf( "## Data type: %s\n", info.glString );
		}
		if ( textureFormat == info.glEnum )
		{
			printf( "## Texture format: %s\n", info.glString );
		}
	}
}

// Steps:
// 1. Load water.bmp
// 2. Upload to GPU as indices and a palette
bool App::CreateTexture()
{
	const auto initialiseTexture = []( const char* textureName, GLuint& handle, const Texture& texture, const GLenum& target, bool indexed = false, bool mipmapping = true )
	{
		std::cout << "CreateTexture: Uploading '" << textureName << "'..." << std::endl;

		glCreateTextures( target, 1, &handle );
		glBindTexture( target, handle );

		if ( GLError( "CreateTexture: Created texture object" ) )
		{
			return false;
		}

		if ( target == GL_TEXTURE_1D )
		{
			glTexImage1D( target, 0,
				indexed ? GL_R8 : GL_RGB8,
				texture.GetWidth(), 0,
				indexed ? GL_RED : GL_RGB,
				GL_UNSIGNED_BYTE,
				texture.GetBuffer().data() );
		}
		else
		{
			glTexImage2D( target, 0,
				indexed ? GL_R8 : GL_RGB8, // internal format
				texture.GetWidth(), texture.GetHeight(), 0, // dimensions
				indexed ? GL_RED : GL_RGB, // format of the data
				GL_UNSIGNED_BYTE, // type of data
				texture.GetBuffer().data() );
		}

		if ( GLError( "CreateTexture: Fed texture object" ) )
		{
			return false;
		}

		if ( target != GL_TEXTURE_1D || !mipmapping )
		{
			glGenerateMipmap( target );
		
			if ( GLError( "CreateTexture: Generated mipmaps" ) )
			{
				return false;
			}
		}

		glTexParameteri( target, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( target, GL_TEXTURE_WRAP_T, GL_REPEAT );

		// GL_LINEAR won't work because we're sampling *indices* from the texture,
		// not the actual pixel values :>
		// Also pay special attention to GL_NEAREST_MIPMAP_NEAREST, it must not be 
		// *_MIPMAP_LINEAR cuz' it'll have wacky consequences like with GL_LINEAR
		// This could be worked around by rendering the result to a texture, but oh well
		glTexParameteri( target, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri( target, GL_TEXTURE_MIN_FILTER, mipmapping ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST );
	
		std::cout << "CreateTexture: uploaded '" << textureName << "' successfully" << std::endl;
		return true;
	};

	{
		texture = TextureProvider::LoadTextureFromFile( "water.bmp" );
		if ( !texture )
		{
			std::cout << "CreateTexture: Could not load image water.bmp" << std::endl;
			return false;
		}

		TextureBuffer paletteTextureBuffer;
		for ( const auto& paletteEntry : texture.GetPalette() )
		{
			paletteTextureBuffer.push_back( paletteEntry[0] );
			paletteTextureBuffer.push_back( paletteEntry[1] );
			paletteTextureBuffer.push_back( paletteEntry[2] );
		}
		paletteTexture = Texture( 256, 1, paletteTextureBuffer, {} );
	}

	if ( !initialiseTexture( "diffuse image", textureHandle, texture, GL_TEXTURE_2D, true ) )
	{
		return false;
	}

	if ( !initialiseTexture( "palette image", paletteTextureHandle, paletteTexture, GL_TEXTURE_2D, false, false ) )
	{
		return false;
	}

	return true;
}

// Just a quad
bool App::CreateGeometry()
{
	// xyz st
	const float quadVerts[] =
	{
		// Bottom-left corner
		-1.0f, -1.0f, 0.0f, 
		0.0f, 0.0f,
		// Bottom-right corner
		1.0f, -1.0f, 0.0f,
		1.0f, 0.0f,
		// Top-right corner
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f,
		// Top-left corner
		-1.0f, 1.0f, 0.0f,
		0.0f, 1.0f
	};

	const uint32_t quadIndices[] =
	{
		0, 1, 2,
		2, 3, 0
	};

	const auto VBOffset = []( const int& offset )
	{
		return reinterpret_cast<void*>(offset);
	};

	// Generate buffer, bind buffer, buffer the buffer
	glGenBuffers( 1, &vertexBufferHandle );
	glBindBuffer( GL_ARRAY_BUFFER, vertexBufferHandle );
	glBufferData( GL_ARRAY_BUFFER, sizeof( quadVerts ), quadVerts, GL_STATIC_DRAW );
	GLError( "CreateGeometry: Created vertex buffer" );

	// Generate VAO, bind VAO, set up vertex attributes
	glGenVertexArrays( 1, &vertexArrayHandle );
	glBindVertexArray( vertexArrayHandle );
	GLError( "CreateGeometry: Created vertex array" );

	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof( float ), VBOffset(0) );
	GLError( "CreateGeometry: Set up vertex position attribute" );
	
	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof( float ), VBOffset(12) );
	GLError( "CreateGeometry: Set up texture coord attribute" );

	// Generate EBO, bind EBO, buffer the EBO
	glGenBuffers( 1, &indexBufferHandle );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexBufferHandle );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( quadIndices ), quadIndices, GL_STATIC_DRAW );
	GLError( "CreateGeometry: Set up index buffer" );

	return true;
}

const char* App::GetShaderError( GLuint vs, GLuint fs ) const
{
	int success;
	static char infoLog[512];

	// Check the vertex shader
	glGetShaderiv( vs, GL_COMPILE_STATUS, &success );
	if ( !success )
	{
		glGetShaderInfoLog( vs, 512, nullptr, infoLog );
		return infoLog;
	}
	// Then the fragment shader
	glGetShaderiv( fs, GL_COMPILE_STATUS, &success );
	if ( !success )
	{
		glGetShaderInfoLog( fs, 512, nullptr, infoLog );
		return infoLog;
	}

	return nullptr;
}

int App::Shutdown( const int& errorCode )
{
	SDL_Quit();

	return errorCode;
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

