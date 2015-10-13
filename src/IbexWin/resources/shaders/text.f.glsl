#version 330 core

#define highp

// Values that stay constant for the whole mesh.
uniform sampler2D textureIn;
uniform vec4 backgroundColor;

// Interpolated values from the vertex shaders
in vec2 UV;
in vec4 outTextColor;

// Ouput data
layout (location = 0) out vec4 color;


void main()
{
    color = mix(backgroundColor, outTextColor, texture(textureIn, UV).r);
}
