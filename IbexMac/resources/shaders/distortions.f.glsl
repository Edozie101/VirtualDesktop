#version 150

#define highp

uniform sampler2D texture;

in vec2 oTexCoord;

layout (location = 0) out vec4 fragColor;

void main()
{
    fragColor = texture2D(texture, oTexCoord);
}
