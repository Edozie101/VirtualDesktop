#version 330

#define highp

layout (location = 0) out vec4 fragColor;

in vec2 UV;

uniform sampler2D textureIn;

void main()
{
    fragColor = texture(textureIn, UV);
}
