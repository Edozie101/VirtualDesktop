#version 330

layout (location = 0) in vec4 a_position;
in vec2 a_texCoord;

out vec2 oTexCoord;

void main()
{
    oTexCoord = a_texCoord.xy;
	gl_Position = a_position;
}
