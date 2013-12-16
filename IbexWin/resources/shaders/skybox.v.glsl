#version 330

in vec3 vertexPosition_modelspace;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

out vec3 UVW;

void main()
{
    vec4 v = vec4(vertexPosition_modelspace,1);
    
    UVW = normalize(v).xyz;
    gl_Position = projectionMatrix*modelViewMatrix*v;
}
