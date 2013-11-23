#version 120

attribute vec4 a_position;
attribute vec2 a_texCoord;

varying vec2 oTexCoord;

void main()
{
    oTexCoord = a_texCoord.xy;
	gl_Position = a_position;
}
