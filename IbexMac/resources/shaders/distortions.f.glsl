#version 120

#define highp

uniform sampler2D texture;

varying vec2 oTexCoord;

void main()
{
    gl_FragColor = texture2D(texture, oTexCoord);
}
