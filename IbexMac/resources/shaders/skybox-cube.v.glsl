#version 330

in vec3 vertexPosition_modelspace;
in vec2 vertexUV;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

out vec2 UV;

void main()
{
    vec4 v = vec4(vertexPosition_modelspace,1);
    
    UV = vertexUV;
    gl_Position = projectionMatrix*modelViewMatrix*v;
}
