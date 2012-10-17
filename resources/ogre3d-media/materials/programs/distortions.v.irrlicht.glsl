#version 120

varying vec2 texcoord;

void main()
{
    gl_Position = gl_Vertex;
    texcoord = gl_MultiTexCoord0.xy;
}
