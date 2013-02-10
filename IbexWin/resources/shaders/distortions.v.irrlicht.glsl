#version 120

attribute vec4 a_position;
attribute vec2 a_texCoord;

uniform float offsetUniform;

void main()
{
    gl_Position = vec4(a_position.x+offsetUniform, a_position.yzw);
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_TexCoord[0].y = 1-gl_TexCoord[0].y;
}
