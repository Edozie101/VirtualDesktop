#version 110

attribute vec4 position;

varying vec2 texcoord;

void main()
{
    gl_Position = position
    texcoord = position * vec2(0.5) + vec2(0.5);
}
