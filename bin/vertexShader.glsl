
#version 330 core

// Render data
layout ( location = 0 ) in vec3 vertexPosition;
layout ( location = 1 ) in vec2 vertexCoord;

// Outputs for the fragment shader
out vec3 fragmentPosition;
out vec2 fragmentCoord;

void main()
{
	gl_Position = vec4( vertexPosition, 1.0 );
	fragmentPosition = vertexPosition;
	fragmentCoord = vertexCoord;
}
