#version 120

attribute vec4 a_position;
attribute vec2 a_texCoord;

varying vec2 texcoord;

void main()
{
    texcoord = a_texCoord.xy; //gl_TexCoord = a_texCoord;//gl_MultiTexCoord0;
	gl_Position = a_position;//gl_Vertex;
}
