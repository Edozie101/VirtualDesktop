#version 330

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;
layout(location = 2) in vec2 vertexUV;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

out vec2 UV;

void main()
{
    vec4 v = vec4(vertexPosition_modelspace,1);
    vec4 v2 = vec4(vertexNormal_modelspace,1);
    
    UV = vertexUV;
    gl_Position = projectionMatrix*(modelViewMatrix*v);
}
