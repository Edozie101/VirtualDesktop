#version 330

#define highp

layout (location = 0) out vec4 fragColor;

in vec3 UVW;

uniform samplerCube cubemap;

void main()
{
    fragColor = texture(cubemap, UVW);
}
