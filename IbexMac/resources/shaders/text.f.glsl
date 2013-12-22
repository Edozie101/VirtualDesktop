#version 330 core

#define highp

// Interpolated values from the vertex shaders
in vec2 UV;

// Ouput data
layout (location = 0) out vec4 color;

// Values that stay constant for the whole mesh.
uniform sampler2D textureIn;
uniform vec4 backgroundColor;
uniform vec4 textColor;

void main()
{
    color = mix(backgroundColor, textColor, texture(textureIn, UV).r);
}
