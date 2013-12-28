#version 330

#define highp

// Interpolated values from the vertex shaders
in vec2 UV;
in vec3 Position_worldspace;
in float outFade;

// Ouput data
layout (location = 0) out vec4 color;

// Values that stay constant for the whole mesh.
uniform sampler2D textureIn;

void main()
{
	color = texture(textureIn, UV)*vec4(1.0f, 1.0f, 1.0f, outFade);
}
