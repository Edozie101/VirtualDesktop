#version 330

#define highp

uniform sampler2D Texture0;
uniform sampler2D lensTexture1;
uniform sampler2D lensTexture2;

in vec2 oTexCoord;

layout (location = 0) out vec4 fragColor;

void main()
{
    fragColor = texture(Texture0, oTexCoord);
}
