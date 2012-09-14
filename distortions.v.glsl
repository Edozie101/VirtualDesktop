#version 120

attribute vec4 a_position;
attribute vec2 a_texCoord;

uniform float offsetUniform;

varying vec2 texcoord;

void main()
{
    gl_Position = vec4(a_position.x+offsetUniform, a_position.yzw);
    texcoord = a_texCoord;
}
